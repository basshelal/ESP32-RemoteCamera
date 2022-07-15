import {createElement, FunctionComponent, RenderableProps} from "preact"
import JSX = createElement.JSX
import {useState} from "preact/compat"

export interface AppProps {

}

export const App: FunctionComponent<AppProps> = (props: RenderableProps<AppProps>): JSX.Element => {

    const [clicks, setClicks] = useState<number>(0)

    const buttonOnClick = () => {
        setClicks((prev) => prev + 1)
    }

    return (<>
        <p>Hello World from Preact!</p>

        <button onClick={buttonOnClick}>Click me!</button>
        <p>Button Clicks: {clicks}</p>
    </>)
}