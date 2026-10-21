## [1.1.0] - 2026-10-21

- `WebPEncoder.encodeRgbaOriented()` encodes RGBA with a `WebPOrientation`
  transform, one of the eight EXIF orientations, applied to its pixels
- `WebPEncodeInput.orientation` carries that transform through `encodeWebP()`
- Windows: the build hook hands the libwebp sources to `cl.exe` in a response
  file, so its command stays under `cmd.exe`'s 8191-character limit

## [1.0.0] - 2025-01-08

- Initial stable release
- `WebPEncoder.encodeRgba()` for lossy WebP encoding with configurable quality
- `WebPEncoder.encodeRgbaLossless()` for lossless WebP encoding
- `WebPEncoder.version` and `WebPEncoder.versionString` for libwebp version info
- `WebPEncodeInput` and `encodeWebP()` for isolate/compute support
- Automatic libwebp source fetching during build
- Cross-platform support (Android, iOS, Linux, macOS, Windows)
