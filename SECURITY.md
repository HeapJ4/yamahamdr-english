# Security

## Supported release

Only the latest preview is maintained. The project translates a legacy Yamaha application; it does not modernize or audit its original audio engine, plug-ins, or online components.

## Reporting a vulnerability

Use the repository's **Security → Report a vulnerability** option when available. Do not post exploit details, credentials, private settings, or sensitive files in a public issue. For ordinary translation problems, use Issues.

## Contribution and release controls

- `main` requires a pull request, resolved review threads, an up-to-date branch, and passing Windows/Linux CI checks from GitHub Actions.
- Force pushes and deletion of `main` are blocked. There are no ruleset bypass actors.
- Version tags are protected against deletion and rewriting.
- Only the repository owner currently has direct repository access. Public contributors use forks and pull requests.
- External contributors' workflow runs require maintainer approval. Actions tokens default to read-only and cannot approve pull requests.
- CODEOWNERS requests the maintainer's review of every change. Required review count is zero so the sole owner can merge their own tested PRs; an outside contributor still cannot merge without repository write access.
- Secret scanning, push protection, and dependency alerts are enabled at the repository level when supported by GitHub.

The source repository excludes Yamaha player binaries. Portable assets are published separately with SHA-256 checksums. Checksum sidecars detect a mismatched download; they are not a digital signature or an independent guarantee of trust.

Repository administrators can change protection settings and release assets. These controls do not protect an account whose credentials have been compromised.
