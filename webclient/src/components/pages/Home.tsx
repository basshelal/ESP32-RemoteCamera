import {FC, JSXElement} from "../../Utils"
import styled from "preact-css-styled"
import {VideoPlayer} from "../ui-elements/VideoPlayer"

export const Home: FC = (): JSXElement => {

    const Root = styled("main",
        ".main { margin-top: 4px } video { width: 100%; margin-left: auto; margin-right: auto }" +
        ".mono { font-family: 'Fira Code', monospace }")
    return (<Root>
        <div className="main">
            <div className="pure-g">
                <p className="pure-u-1-3 mono" style={{textAlign: "left"}}>Battery: 69% (3.89V)</p>
                <p className="pure-u-1-3 mono" style={{textAlign: "center"}}>Info: Info 1</p>
                <p className="pure-u-1-3 mono" style={{textAlign: "right"}}>Info: Info 2</p>
            </div>

            <VideoPlayer videoOptions={{autoPlay: false, controls: true, muted: true}}/>
        </div>
    </Root>)
}