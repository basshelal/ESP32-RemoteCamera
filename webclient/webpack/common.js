const HtmlWebpackPlugin = require("html-webpack-plugin")
const path = require("path")
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const CssMinimizerPlugin = require("css-minimizer-webpack-plugin");

const srcDir = path.join(__dirname, "..", "src");

module.exports = {
    entry: {
        main: path.join(srcDir, "Main.tsx"),
    },
    output: {
        path: path.join(__dirname, "../../webpages/"),
        filename: "[name].js",
        publicPath: ""
    },
    optimization: {
        minimize: false,
        usedExports: true,
        minimizer : [
            `...`,
            new CssMinimizerPlugin(),
        ]
    },
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: "ts-loader",
                exclude: /node_modules/,
            },
            {
                test: /\.css$/,
                use: [
                    MiniCssExtractPlugin.loader,
                    "css-loader"
                ]
            }
        ],
    },
    resolve: {
        extensions: [".ts", ".tsx", ".js", ".jsx"],
    },
    plugins: [
        new HtmlWebpackPlugin({
            template: "public/index.html",
            inject: "body",
            minify: "auto",
        }),
        new MiniCssExtractPlugin({
            filename: "[name].css",
            chunkFilename: "[id].css"
        }),
    ],
};