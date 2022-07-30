import {App} from "./components/App"
import {render} from "preact"
import "../public/pure.css"
import {Constants} from "./Utils"

Constants.ServerURLHost = new URL(document.URL).origin
render(<App/>, document.getElementById("main")!)
