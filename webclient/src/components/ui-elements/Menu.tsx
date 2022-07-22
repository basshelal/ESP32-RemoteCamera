import {FunctionComponent, JSX} from "preact"

export const Menu: FunctionComponent = (props): JSX.Element => {
    return (<>
        <ul>
            <li>Home</li>
            <li>Files</li>
            <li>Settings</li>
        </ul>
    </>)
}