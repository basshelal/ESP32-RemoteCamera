import HtmlWebpackPlugin from "html-webpack-plugin";
import path from "path";
import {fileURLToPath} from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const srcDir = path.join(__dirname, "..", "src");

export default {
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
    },
    module: {
        rules: [{
            test: /\.tsx?$/,
            use: "ts-loader",
            exclude: /node_modules/,
        }],
    },
    resolve: {
        extensions: [".ts", ".tsx", ".js", ".jsx"],
    },
    plugins: [
        // new CopyPlugin({
        //     patterns: [{from: "./public/", to: "../webpages"}]
        // }),
        new HtmlWebpackPlugin({
            template: "public/index.html",
            inject: "body",
            minify: "auto",
        }),
    ],
};

// TODO: Plugin to minify materialize.css and inline it into index.html and minify index.html