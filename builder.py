from typing import Generator, Iterable
import re, os, sys
import yaml

class SourceGraph:
  def __init__(self, include_dirs: list[str], source_dirs: list[str], links: list[str], options: list[str], builddir: str, runs: dict[str, str]):
    self.load_files(include_dirs, source_dirs)
    self.setup_compiler(links, options)
    self.compute_dependencies()
    self.assemble(builddir)
    self.append_runs(runs)

  def setup_compiler(self, links, options):
    self.links = links
    self.options = options
    self.cc = ("g++ %s" %
                   " ".join(["-I%s" % _ for _ in self.include_dirs]
                            + ["-l%s" % _ for _ in self.links]
                            + ["%s" % _ for _ in self.options]))

  def get_all_files(self, dir: str) -> Generator[str, None, None]:
    for entry in os.listdir(dir):
      path = os.path.join(dir, entry)
      if os.path.isfile(path):
        yield os.path.normcase(path)
      elif os.path.isdir(path):
        for _ in self.get_all_files(path):
          yield _

  def filter_by_extension(self, files: Iterable[str], ext: str|list[str]) -> Generator[str, None, None]:
    if isinstance(ext, str):
      ext = [ext]
    for path in files:
      if os.path.splitext(path)[-1] in ext:
        yield path

  def extract_dependencies(self, path: str) -> Generator[str, None, None]:
    local_dir = os.path.dirname(path)
    with open(path, mode="r", encoding="utf-8") as file:
      text = file.read()
      for gdep in re.findall("#include\\s*<([a-zA-Z_./]+)>", text):
        candidate = os.path.normcase(gdep)
        if candidate in self.includes:
            yield candidate
        for dir in self.include_dirs:
            candidate = os.path.normcase(os.path.join(dir, gdep))
            if candidate in self.includes:
                yield candidate
      for ldep in re.findall('#include\\s*"([a-zA-Z_./]+)"', text):
        candidate = os.path.normcase(os.path.join(local_dir, ldep))
        if candidate in self.includes:
            yield candidate
    
  def load_files(self, include_dirs: list[str], source_dirs: list[str]):
    self.include_dirs = include_dirs
    self.source_dirs = source_dirs
    self.includes = []
    self.sources = []
    for dir in include_dirs:
      self.includes += list(self.filter_by_extension(self.get_all_files(dir), [".hh", ".hpp", ""]))
    for dir in source_dirs:
      self.sources += list(self.filter_by_extension(self.get_all_files(dir), [".cc", ".cpp"]))
  
  def compute_dependencies(self):
    self.dependencies : dict[str, set[str]] = {}
    
    for path in self.includes:
      self.dependencies[path] = set()
      for dep in self.extract_dependencies(path):
        self.dependencies[path].add(dep)
    
    for path in self.sources:
      self.dependencies[path] = set()
      for dep in self.extract_dependencies(path):
        self.dependencies[path].add(dep)
        self.dependencies[path].update(self.dependencies[dep])
  
  def add_object_rule(self, source, object, dependencies):
    print("%s: %s\n\t%s -o %s -c %s\n" % (
      object, " ".join([source]+dependencies),
      self.cc,
      object, source
    ))
  
  def add_executable_rule(self, executable, objects):
    print("%s: %s\n\t%s -o %s %s\n" % (
      executable, " ".join(objects),
      self.cc,
      executable, " ".join(objects)
    ))

  def default_hook(self, executable: str):
    print("@all: %s\n" % executable)

  def run_rule(self, name: str, executable: str, arguments: str):
    print("%s: %s\n\t./%s %s\n" % (name, executable, executable, arguments))

  def clean_rule(self, dirs: str):
    print("clean:")
    print("\trm -rf builddir")
    for dir in sorted(list(dirs)):
        print("\tmkdir %s" % dir)
    print()
  
  def assemble(self, builddir: str):
    self.builddir = builddir
    objects = []
    executable = os.path.normcase(os.path.join(self.builddir, "main.exe"))
    self.executable = executable
    self.default_hook(executable)
    for source in self.sources:
      object = os.path.normcase(os.path.join(builddir, source.replace(".cc", ".o").replace(".cpp", ".o")))
      objects.append(object)
      dependencies = list(self.dependencies[source])
      self.add_object_rule(source, object, dependencies)
    self.add_executable_rule(executable, objects)
    self.clean_rule(set([os.path.dirname(_) for _ in objects+[executable]]))
    self.run_rule("run", executable, "")

  def append_runs(self, runs: dict[str, str]):
    self.runs = runs
    for key in self.runs.keys():
        self.run_rule(key, self.executable, self.runs[key])

  @staticmethod
  def from_config(config_path: str):
      with open(config_path, mode="r", encoding="utf-8") as file:
          doc = yaml.safe_load(file)
          return SourceGraph(
            include_dirs=(doc.get('include_dirs') or []),
            source_dirs=(doc.get('source_dirs') or []),
            links=(doc.get('links') or []),
            options=(doc.get('options') or []),
            builddir=(doc.get('builddir') or 'builddir'),
            runs=(doc.get('runs') or {})
          )

if __name__ == "__main__":
  SourceGraph.from_config('build.yml')
