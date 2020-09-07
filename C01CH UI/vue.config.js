const CompressionWebpackPlugin  = require('compression-webpack-plugin');
const { CleanWebpackPlugin } = require('clean-webpack-plugin');
const path = require('path');

module.exports = {
  productionSourceMap: false,
  "transpileDependencies": [
    "vuetify"
  ],
  configureWebpack: {
    plugins: [
      new CompressionWebpackPlugin({
        cache: false,
        algorithm: 'gzip',
        deleteOriginalAssets: process.env.NODE_ENV === 'production',
      }),
      new CleanWebpackPlugin({
        cleanOnceBeforeBuildPatterns: [path.join(process.cwd(), '../C01CH fw/data/*.*')],
        dry: false,
        dangerouslyAllowCleanPatternsOutsideProject: true
      })
    ],
    output:{
      filename: '[name].[hash:4].js',
      chunkFilename: '[name].[hash:4].js'
    },
    optimization: {
      splitChunks: false
    }
  },
  outputDir: process.env.NODE_ENV === 'production'
    ? path.join(process.cwd(), '../C01CH fw/data')
    : 'dist',
  css: {
    extract: false,
  },
}