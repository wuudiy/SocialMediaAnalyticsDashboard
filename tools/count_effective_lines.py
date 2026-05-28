from pathlib import Path
from collections import defaultdict

CODE_EXTENSIONS = {
    ".cpp", ".h", ".hpp", ".c", ".cc", ".cxx", ".cmake"
}

SPECIAL_FILES = {
    "CMakeLists.txt"
}

IGNORE_DIRS = {
    ".git",
    ".idea",
    ".vs",
    ".vscode",
    ".qtcreator",
    "build",
    "cmake-build-debug",
    "cmake-build-release",
    "CMakeFiles",
    "debug",
    "release",
}

def is_code_file(path: Path) -> bool:
    return path.suffix in CODE_EXTENSIONS or path.name in SPECIAL_FILES

def should_ignore(path: Path) -> bool:
    parts = set(path.parts)
    return bool(parts & IGNORE_DIRS)

def count_effective_lines(path: Path) -> int:
    count = 0
    in_block_comment = False

    try:
        lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
    except Exception:
        return 0

    for line in lines:
        text = line.strip()

        if not text:
            continue

        # CMake 注释
        if path.suffix == ".cmake" or path.name == "CMakeLists.txt":
            if text.startswith("#"):
                continue
            count += 1
            continue

        # C / C++ 块注释处理
        while True:
            if in_block_comment:
                end = text.find("*/")
                if end == -1:
                    text = ""
                    break
                text = text[end + 2:].strip()
                in_block_comment = False

            start = text.find("/*")
            single = text.find("//")

            # 单行注释在块注释之前，截断后面的注释
            if single != -1 and (start == -1 or single < start):
                text = text[:single].strip()
                break

            if start == -1:
                break

            end = text.find("*/", start + 2)

            if end == -1:
                text = text[:start].strip()
                in_block_comment = True
                break

            text = (text[:start] + text[end + 2:]).strip()

        if text:
            count += 1

    return count

def main():
    root = Path.cwd()

    total = 0
    by_dir = defaultdict(int)
    by_ext = defaultdict(int)
    file_rows = []

    for path in root.rglob("*"):
        if not path.is_file():
            continue

        if should_ignore(path):
            continue

        if not is_code_file(path):
            continue

        loc = count_effective_lines(path)
        total += loc

        top_dir = path.relative_to(root).parts[0]
        ext = path.name if path.name == "CMakeLists.txt" else path.suffix

        by_dir[top_dir] += loc
        by_ext[ext] += loc
        file_rows.append((str(path.relative_to(root)), loc))

    print("=" * 70)
    print("Effective Code Lines Summary")
    print("=" * 70)
    print(f"Total effective code lines: {total}")
    print()

    print("By directory:")
    for name, loc in sorted(by_dir.items(), key=lambda x: x[0]):
        print(f"  {name:<25} {loc:>6}")

    print()
    print("By file type:")
    for name, loc in sorted(by_ext.items(), key=lambda x: x[0]):
        print(f"  {name:<25} {loc:>6}")

    print()
    print("By file:")
    for file_name, loc in sorted(file_rows, key=lambda x: x[0]):
        print(f"  {file_name:<60} {loc:>6}")

if __name__ == "__main__":
    main()
