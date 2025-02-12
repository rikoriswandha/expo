import { TransformPipeline } from '.';

export function podspecTransforms(versionName: string): TransformPipeline {
  return {
    transforms: [
      // Common transforms
      {
        // Transforms some podspec fields by adding versionName at the beginning
        replace: /\.(name|header_dir|module_name)(\s*=\s*["'])([^"']+)(["'])/g,
        with: `.$1$2${versionName}$3$4`,
      },
      {
        // Prefixes dependencies listed in the podspecs
        replace: /(\.dependency\s+["'])(Yoga|React\-|ReactCommon|RCT|FB)(?!-Folly)([^"']*["'])/g,
        with: `$1${versionName}$2$3`,
      },
      {
        // Removes source conditional
        replace: /source\s*\=\s*\{[.\S\s]+?end/g,
        with: '',
      },
      {
        // Points spec source at correct directory
        replace: /(\.source\s*\=\s*)\S+\n/g,
        with: '$1{ :path => "." }\n',
      },

      // React-Core & ReactCommon
      {
        // Fixes header_subspecs for RCTBlobHeaders
        paths: 'React-Core.podspec',
        replace: /\{(RCTBlobManager),(RCTFileReaderModule)\}/g,
        with: `{${versionName}$1,${versionName}$2}`,
      },
      {
        // Prefixes conflicting AccessibilityResources
        paths: 'React-Core.podspec',
        replace: /"AccessibilityResources"/g,
        with: `"${versionName}AccessibilityResources"`,
      },
      {
        // Fixes HEADER_SEARCH_PATHS
        paths: ['React-Core.podspec', 'ReactCommon.podspec'],
        replace: /(Headers\/Private\/)(React-Core)/g,
        with: `$1${versionName}$2`,
      },

      // React-cxxreact
      {
        // Fixes excluding SampleCxxModule.* files
        paths: 'React-cxxreact.podspec',
        replace: /\.exclude_files(\s*=\s*["'])(SampleCxxModule\.\*)(["'])/g,
        with: `.exclude_files$1${versionName}$2$3`,
      },

      // Yoga
      {
        // Unprefixes inner directory for source_files
        paths: 'Yoga.podspec',
        replace: /\{(Yoga),(YGEnums),(YGMacros),(YGNode),(YGStyle),(YGValue)\}/g,
        with: `{${versionName}$1,${versionName}$2,${versionName}$3,${versionName}$4,${versionName}$5,${versionName}$6}`,
      },

      // FBReactNativeSpec
      {
        // Fixes HEADER_SEARCH_PATHS
        paths: 'FBReactNativeSpec.podspec',
        replace: /(\/Libraries\/)(FBReactNativeSpec)/g,
        with: `$1${versionName}$2`,
      },
      {
        // Disable codegen from build phase script
        paths: 'FBReactNativeSpec.podspec',
        replace: /(use_react_native_codegen!)/g,
        with: '# $1',
      },
    ],
  };
}
