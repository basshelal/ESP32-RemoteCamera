const merge = require("webpack-merge").merge
const common = require("./common");

module.exports = merge(common, {
    devtool: "inline-source-map",
    mode: "development",
    optimization: {
        minimize: false,
    },
    devServer: {
        port: 3000,
        historyApiFallback: true,
        hot: true
    },
});