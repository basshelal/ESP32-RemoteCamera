import {FunctionComponent, JSX} from "preact"

export const Header: FunctionComponent = (props): JSX.Element => {
    return (<>
        <ul style={{}}>
            <li>04:20:42 Friday 20 April 2022</li>
            <li>69% (3.8V)</li>
            <li>Log Out</li>
        </ul>
    </>)
}