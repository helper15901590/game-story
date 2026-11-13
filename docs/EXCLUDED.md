# 发布版未包含的内容

为保护第三方版权、商业信息与仓库体积，公开版本不包含以下内容。完整开发目录中仍保留这些文件，维护者可自行决定是否另行发布。

## 音频与视频

- 第三方配音与故事音频（如"晚安妈妈"系列、《小红帽》《村庄任务》样音）
- TF 卡音乐库、`TF卡文件.zip`
- 烧录 / 写卡教学视频（`.mp4`）

原因：音频版权与邻接权未确认。请使用自制、AI 生成且可商用，或已获授权的音频。

## 商业与隐私资料

- `investor_pitch.md`、家长需求调研、报价单、销售与供应链资料
- BOM（含供应商名称、联系人、电话、价格）
- 结构件 ZIP（完整结构设计，单个文件可达 40 MB）

原因：商业敏感信息与仓库体积。

## 第三方固件与文档

- WT2003H4-16S 语音芯片固件 `.fw`（版权归唯创，需向供应商获取）
- 结构 / 模具源文件
- 带品牌水印的供应商规格图

原因：再分发权限不明确。WT2003 芯片手册仅作技术参考，版权归原厂。

## 如何补充

- **音频**：按 README 第 5.2 节的命名规则制作，放入 `SYSTE/` 与故事文件夹。
- **BOM**：可整理一份不含价格与联系人的版本再提交。
- **结构**：如需开源，建议使用 CERN-OHL 等硬件许可证单独发布。

---

# Not included in this release

To protect third-party copyrights, commercial information, and repository size, the public release omits:

- Third-party voice recordings and story audio (including the *Little Red Riding Hood* and *Village Quest* samples)
- The TF-card audio library and `TF卡文件.zip`
- Flashing and card-writing tutorial videos (`.mp4`)
- `investor_pitch.md`, parent research, quotes, sales/supply-chain documents
- The BOM (contains supplier names, contacts, phone numbers, and prices)
- Enclosure ZIPs (up to 40 MB each)
- The WT2003H4-16S voice-chip firmware `.fw` (copyright Waytronic; obtain from the supplier)
- Enclosure/mold source files and watermarked supplier drawings

Use self-produced, commercially licensed, or otherwise authorized audio. For a public BOM, remove prices and supplier contacts first. For open hardware, consider a dedicated hardware license such as CERN-OHL.
