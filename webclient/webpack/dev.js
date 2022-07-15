import {merge} from "webpack-merge"
import common from "./common.js";

export default merge(common, {
    devtool: "inline-source-map",
    mode: "development",
    optimization: {
        minimize: false,
    }
});