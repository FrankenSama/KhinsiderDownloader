# Contributing to KhinsiderDownloader

This is a personal project maintained by Octavio Silva (FrankenSama). Forks are welcome under the GPLv3, and bug reports or focused pull requests are appreciated. Reviews and merges happen at the maintainer's discretion, so there is no guaranteed response time.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Forking & Attribution](#forking--attribution)
- [Development Setup](#development-setup)
- [Coding Standards](#coding-standards)
- [Commit Guidelines](#commit-guidelines)
- [Pull Request Process](#pull-request-process)
- [Recognition](#recognition)
- [Questions?](#questions)
- [License](#license)

## Code of Conduct

This project and everyone participating in it is governed by our commitment to a welcoming, respectful community. By participating, you are expected to uphold this code. Please report unacceptable behavior by opening an issue or contacting the maintainer through GitHub.

### Our Standards

- Using welcoming and inclusive language
- Being respectful of differing viewpoints and experiences
- Gracefully accepting constructive criticism
- Focusing on what is best for the community
- Showing empathy towards other community members

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the existing issues as you might find that you don't need to create one. When you are creating a bug report, please include as many details as possible:

**Bug Report Template:**

```markdown
**Describe the bug**
A clear and concise description of what the bug is.

**To Reproduce**
Steps to reproduce the behavior:
1. Go to '...'
2. Click on '....'
3. Scroll down to '....'
4. See error

**Expected behavior**
A clear and concise description of what you expected to happen.

**Screenshots**
If applicable, add screenshots to help explain your problem.

**Environment:**
 - OS: [e.g. Windows 11, Ubuntu 24.04]
 - App version: [e.g. 3.1.0.44]
 - Channel: [Stable or Nightly]
 - Download folder / thread settings, if relevant

**Logs**
If logging is enabled, attach the relevant lines from the log folder.

**Additional context**
Add any other context about the problem here.
```

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, please include:

- **Use a clear and descriptive title**
- **Provide a detailed description** of the suggested enhancement
- **Explain why this enhancement would be useful** to most users
- **List some examples** of how this enhancement would be used
- **Specify which version** you're using

### Pull Requests

1. Fork the repo and create your branch from `dev`
2. If you've added code that should be tested, add or update tests
3. Ensure your code follows the existing style
4. Update the README.md or docs with details of changes if needed
5. Open the pull request against `dev`; it will be merged once the maintainer has reviewed it

Development happens on `dev`, and changes reach `master` only when they are ready for a stable release. Please do not bump the app version or change release flags in a normal pull request — version bumps and releases are handled by the maintainer.

Pull requests are optional. If you would rather maintain your own version, fork the repository and follow the attribution guidance below.

## Forking & Attribution

Forking is welcome. If you publish a modified version:

- Keep the `LICENSE` file and copyright notices (required by the GPLv3).
- Credit Weespin and Octavio Silva, and link back to this repository in your README or about page (requested).
- Make it clear that your fork is not the official build, and don't use the project name or icon in a way that suggests it is.

GitHub's **Fork** button preserves the license and shows a "forked from" link automatically.

## Development Setup

### Prerequisites

- **CMake 3.21+**
- **Qt 6.12+** (MSVC 2022 64-bit build on Windows)
- **vcpkg** with `curl` and `libxml2`
- A **C++17** compiler (Visual Studio 2022 on Windows, GCC/Clang on Linux)
- Git

### Setting Up Your Development Environment

1. **Fork and Clone**

   ```bash
   git clone https://github.com/YOUR-USERNAME/KhinsiderDownloader.git
   cd KhinsiderDownloader
   ```

2. **Create a Branch**

   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Configure and Build (Windows)**

   vcpkg is easiest at `C:\vcpkg`; run `bootstrap-vcpkg.bat` once if you have not already.

   ```bat
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 ^
     -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
     -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64
   cmake --build . --config Release
   ```

   Alternatively, `scripts/windows/fullbuild.bat` configures, builds, and deploys in one step.

4. **Configure and Build (Linux)**

   ```bash
   sudo apt install qt6-base-dev cmake build-essential qtdeclarative5-dev \
     qt6-declarative-dev libqt6xml6 libcurl4-openssl-dev \
     qml6-module-qt5compat-graphicaleffects
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   cd ..
   chmod +x scripts/linux/deploy.sh
   ./scripts/linux/deploy.sh
   ```

5. **Run the Parser Tests (optional)**

   The parser tests are opt-in and disabled by default. Enable them at configure time, then run `ctest`:

   ```bash
   cmake .. -DKHINSIDER_BUILD_TESTS=ON
   cmake --build . --config Release
   ctest -C Release --output-on-failure
   ```

   On Windows, `ctest` needs Qt and the vcpkg DLLs on `PATH` (for example `C:\Qt\6.12.0\msvc2022_64\bin` and `C:\vcpkg\installed\x64-windows\bin`).

6. **Test Thoroughly**

   - Launch the deployed build, not just the raw executable, so the Qt runtime and DLLs resolve
   - Try a real search and a small download
   - Check the Settings and About pages
   - Resize the window and restart to confirm settings persist
   - Watch for warnings in the log folder if logging is enabled

## Coding Standards

The project is C++17 with Qt 6 and QML. Keep changes focused and consistent with the surrounding code rather than rewriting working modules.

### C++ Style Guide

Follow the existing Qt-flavoured C++ conventions:

```cpp
// ✅ Good: clear ownership, Qt types, const-correct
QString KhinsiderParser::buildAlbumUrl(const QString &albumId) const
{
    return QStringLiteral("https://downloads.khinsider.com/album/%1").arg(albumId);
}

// ❌ Bad: manual memory management, no const, unclear name
char *getUrl(std::string id)
{
    char *buf = new char[256];
    sprintf(buf, "https://downloads.khinsider.com/album/%s", id.c_str());
    return buf;
}
```

### Key Principles

1. **Use descriptive names** — `albumUrl`, `downloadedSongs`, and `pendingSongs` beat `u`, `n`, and `p`.
2. **Prefer Qt types** — `QString`, `QList`, `QUrl`, and `QNetworkAccessManager` fit the rest of the codebase.
3. **Respect ownership** — use Qt parent/child ownership or smart pointers; avoid raw `new`/`delete`.
4. **Keep functions small and focused** — each function should do one thing well.
5. **Comment complex logic** — explain *why*, not *what*, especially around parsing and download scheduling.
6. **Handle errors** — check parser results and network replies; surface failures to the UI instead of silently swallowing them.
7. **Don't block the UI thread** — downloads and parsing belong off the GUI thread or in async workers.

### QML Style Guide

- Use the shared `Theme` palette (`Theme.bg`, `Theme.accent`, and friends) instead of hard-coded colors.
- Reuse the shared components in `src/ui/shared` (`WButton`, `WScrollView`, and similar) before adding new ones.
- Keep layouts responsive; the window is resizable and has a minimum size of 1000×600.
- Give interactive controls an `id` and wire actions through the existing controllers rather than reaching into C++ directly.

```qml
// ✅ Good: themed, anchored, reuses shared components
WButton {
    Layout.preferredWidth: 110
    Layout.preferredHeight: 40
    label: qsTr("Search")
    onClicked: mainWindow.performSearch()
}

// ❌ Bad: hard-coded colors and sizes, no translation
Rectangle {
    width: 110; height: 40; color: "#4a90d9"
    MouseArea { anchors.fill: parent; onClicked: app.searchController.doSearch("x") }
}
```

### File Organization

```text
KhinsiderDownloader/
├── src/
│   ├── main.cpp              # Entry point
│   ├── App.cpp / App.h       # Application wiring and context objects
│   ├── Theme.h               # Shared colour palette
│   ├── parser/               # Khinsider HTML/XML parsing
│   ├── downloader/           # Async download engine and worker pool
│   ├── pages/                # Controllers and models for each page
│   └── ui/
│       ├── Main.qml          # Window, sidebar, and tab switching
│       ├── search/           # Search page
│       ├── download/         # Download queue page
│       ├── settings/         # Settings page
│       ├── about/            # About page and update checker
│       └── shared/           # Reusable QML components
├── tests/                    # Opt-in parser tests
├── scripts/windows/          # Windows build/deploy scripts
├── scripts/linux/            # Linux build/deploy scripts
├── docs/images/              # README screenshots
├── CMakeLists.txt
├── vcpkg.json
├── CONTRIBUTING.md
├── LICENSE                   # GPLv3
└── README.md
```

## Commit Guidelines

### Commit Message Format

We follow the [Conventional Commits](https://www.conventionalcommits.org/) specification:

```text
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: A new feature
- `fix`: A bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, whitespace, etc.)
- `refactor`: Code refactoring without changing functionality
- `perf`: Performance improvements
- `test`: Adding or updating tests
- `chore`: Maintenance tasks, dependencies, and CI

### Examples

```bash
feat(search): add keyboard navigation to result list

Allow arrow keys to move through results and Enter to open the
selected album, matching the search field's existing shortcut.

Closes #123
```

```bash
fix(downloader): keep queue count in sync after cancellation

Cancelled songs were still counted as pending, so the sidebar badge
and window title never reached zero. Update the totals after each
cancellation.

Fixes #456
```

```bash
docs(readme): restyle readme and add contributing guide
```

### Release Flags

Commit subjects such as `[RELEASE]` and `[PRE-RELEASE]` are reserved for the maintainer's release workflow on `master`; they trigger version bumps and publishing. Please don't include them in pull request commits. `[skip-ci]` is likewise for maintainer use only.

## Pull Request Process

### Before Submitting

- [ ] Code follows the project's style guidelines
- [ ] Comments added for complex logic
- [ ] No debug logging or leftover commented-out code
- [ ] Project builds locally in `Release`
- [ ] Parser tests pass if you enabled them (`-DKHINSIDER_BUILD_TESTS=ON`)
- [ ] Tested the deployed build, including a search and a small download
- [ ] No breaking changes (or clearly documented if unavoidable)
- [ ] Documentation updated if needed
- [ ] Version files (`APP_VERSION`) and release flags left untouched

### PR Template

```markdown
## Description
Brief description of what this PR does

## Type of Change
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update

## Testing
Describe the tests you ran and how to reproduce them

## Screenshots (if applicable)
Add screenshots showing the changes

## Checklist
- [ ] My code follows the style guidelines
- [ ] I have commented my code, particularly in hard-to-understand areas
- [ ] I have made corresponding changes to the documentation
- [ ] My changes generate no new warnings
- [ ] I have tested the deployed build
```

### Review Process

1. The maintainer will review the PR when time allows
2. Address any requested changes
3. Once approved, the PR will be merged into `dev` and included in a future release

## Recognition

Accepted contributions are credited in release notes. Thank you for helping make KhinsiderDownloader better! 🎉

## Questions?

Open an issue on GitHub if you have a question about the project.

## License

By contributing, you agree that your contributions will be licensed under the GNU General Public License v3.0. You retain copyright to your own contributions.

---

**Thank you for helping improve KhinsiderDownloader!** 🎵
