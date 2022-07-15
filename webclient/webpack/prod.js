import common from "./common.js";
import {merge} from "webpack-merge";
import HtmlInlineScriptPlugin from "html-inline-script-webpack-plugin";

export default merge(common, {
    mode: "production",
    optimization: {
        minimize: true
    },
    plugins: [
        new HtmlInlineScriptPlugin()
    ]
});