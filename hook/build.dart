import 'dart:io';

import 'package:hooks/hooks.dart';
import 'package:logging/logging.dart';
import 'package:native_toolchain_c/native_toolchain_c.dart';

const _libwebpRepo = 'https://github.com/swipelab/libwebp.git';

String _getCacheDir() {
  if (Platform.isWindows) {
    // Use LOCALAPPDATA on Windows (e.g., C:\Users\<user>\AppData\Local)
    final localAppData = Platform.environment['LOCALAPPDATA'];
    if (localAppData != null) {
      return '$localAppData\\swipelab_webp';
    }
    // Fallback to USERPROFILE
    final userProfile = Platform.environment['USERPROFILE'] ?? '';
    return '$userProfile\\.cache\\swipelab_webp';
  }
  // Unix-like systems (macOS, Linux)
  final home = Platform.environment['HOME'] ?? '';
  return '$home/.cache/swipelab_webp';
}

Future<String> _ensureLibwebp(Logger logger) async {
  final cacheDir = _getCacheDir();
  final separator = Platform.isWindows ? '\\' : '/';
  final libwebpPath = '$cacheDir${separator}libwebp';

  if (Directory(libwebpPath).existsSync()) {
    logger.info('libwebp found at $libwebpPath');
    return libwebpPath;
  }

  logger.info('libwebp not found, cloning from $_libwebpRepo...');

  // Create cache directory if it doesn't exist
  await Directory(cacheDir).create(recursive: true);

  // Clone libwebp
  final result = await Process.run(
    'git',
    ['clone', '--depth', '1', _libwebpRepo, libwebpPath],
    workingDirectory: cacheDir,
  );

  if (result.exitCode != 0) {
    throw Exception(
      'Failed to clone libwebp: ${result.stderr}\n'
      'Please ensure git is installed and you have internet access.\n'
      'Alternatively, manually clone $_libwebpRepo to $libwebpPath',
    );
  }

  logger.info('libwebp cloned successfully to $libwebpPath');
  return libwebpPath;
}

void main(List<String> args) async {
  await build(args, (input, output) async {
    final logger = Logger('')
      ..level = Level.ALL
      ..onRecord.listen((record) => print(record.message));

    final packageName = input.packageName;

    // Ensure libwebp is available (clone if needed)
    final libwebpPath = await _ensureLibwebp(logger);

    // Collect all libwebp source files
    final webpSources = <String>[
      // Our wrapper
      'src/swipelab_webp.c',
      // sharpyuv
      ...Directory('$libwebpPath/sharpyuv')
          .listSync()
          .whereType<File>()
          .where((f) => f.path.endsWith('.c'))
          .map((f) => f.path),
      // src/enc
      ...Directory('$libwebpPath/src/enc')
          .listSync()
          .whereType<File>()
          .where((f) => f.path.endsWith('.c'))
          .map((f) => f.path),
      // src/dsp
      ...Directory('$libwebpPath/src/dsp')
          .listSync()
          .whereType<File>()
          .where((f) => f.path.endsWith('.c'))
          .map((f) => f.path),
      // src/utils
      ...Directory('$libwebpPath/src/utils')
          .listSync()
          .whereType<File>()
          .where((f) => f.path.endsWith('.c'))
          .map((f) => f.path),
    ];

    final builder = CBuilder.library(
      name: packageName,
      assetName: 'swipelab_webp.dart',
      sources: webpSources,
      includes: [
        libwebpPath,
        '$libwebpPath/src',
      ],
      flags: [
        '-DWEBP_USE_THREAD=1',
        '-DWEBP_NEAR_LOSSLESS=1',
      ],
      // Link against libm for math functions (not needed on Windows)
      libraries: Platform.isWindows ? [] : ['m'],
    );

    await builder.run(
      input: input,
      output: output,
      logger: logger,
    );
  });
}
