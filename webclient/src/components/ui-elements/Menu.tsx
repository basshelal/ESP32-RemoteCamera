import {Element, FC} from "../../Utils"

export const List: FC = (props): Element => {
    return (<ul style={{}}>
    </ul>)
}

export const Menu: FC = (props): Element => {
    return (<>
        <ul>
            <li>Home</li>
            <li>Files</li>
            <li>Settings</li>
        </ul>
    </>)
}