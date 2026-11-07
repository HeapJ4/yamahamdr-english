# Contributing

Keep changes focused and run `python -m unittest discover -s tests -v` before opening a pull request.

Translations live in `translations.txt`, with one `Japanese|English` entry per line. A literal `\n` represents a line break. Preserve leading/trailing spaces, `%` placeholders, and accelerator letters where applicable. Shortcut suffixes after a tab are preserved automatically. Use short English labels that fit the existing dialog controls. Font names and control classes are structural data and are deliberately preserved.

Changing translation coverage also requires reviewing the expected counts in `supported-build.json`. Do not add an unsupported binary hash without checking its resources and testing a local build. Avoid committing Yamaha binaries, generated images, personal paths, logs, credentials, and local build folders.

Useful issue reports include Windows version and architecture, MidRadio version, exact reproduction steps, whether the machine had an existing installation, and a cropped screenshot if a label is clipped or untranslated.
