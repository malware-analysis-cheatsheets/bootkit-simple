# Bootkit
Windows用のBootkitを作成する。
`ntoskrnl.exe`をロードする前にドライバをマッピングすることで、`DSE(Driver Signature Enforcement)`を回避する。
このリポジトリは[btbd/umap](https://github.com/btbd/umap)と[memN0ps/bootkit-rs](https://github.com/memN0ps/bootkit-rs)を参考に実装している。

## クレジット/参考
- https://github.com/pbatard/uefi-simple
- https://github.com/btbd/umap
- https://github.com/memN0ps/bootkit-rs
- https://github.com/Cr4sh/s6_pcie_microblaze
