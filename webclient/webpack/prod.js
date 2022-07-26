const common = require("./common")
const merge = require("webpack-merge").merge
const HtmlInlineScriptPlugin = require("html-inline-script-webpack-plugin")
const HTMLInlineCSSWebpackPlugin = require("html-inline-css-webpack-plugin").default

module.exports = merge(common, {
    mode: "production",
    optimization: {
        minimize: true
    },
    plugins: [
        new HtmlInlineScriptPlugin(),
        new HTMLInlineCSSWebpackPlugin({
            styleTagFactory({ style }) {
                return `<style>${style}</style>`
            },
            replace: {
                target: "<link rel=\"stylesheet\" href=\"pure.css\"/>",
                removeTarget: true
            }
        }),
    ]
});