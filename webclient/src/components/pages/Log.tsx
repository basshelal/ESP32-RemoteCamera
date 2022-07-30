import {FC, JSXElement} from "../../Utils"
import {LogView} from "../ui-elements/LogView"
import styled from "preact-css-styled"

export const Log: FC = (): JSXElement => {
    const Root = styled("main",
        ".main{position:relative}" +
        ".content{overflow-y:scroll;-webkit-overflow-scrolling:touch;position:fixed;height:clamp(50vh,80vh,90vh);width:90%}")

    return (<Root>
        <div className="main">
            <h1>Log</h1>
            <LogView className="content"/>
        </div>
    </Root>)
}