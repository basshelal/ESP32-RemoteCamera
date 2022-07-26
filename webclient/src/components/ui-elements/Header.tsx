import {FC, JSXElement} from "../../Utils"
import {useContext, useEffect} from "preact/hooks"
import {AppContext, AppContextObject} from "../AppContext"
import styled from "preact-css-styled"
import classNames from "classnames"

export const Header: FC = (props, context): JSXElement => {

    const appContext: AppContextObject = useContext<AppContextObject>(AppContext)

    useEffect(() => {
        const element = document.getElementById(`MenuItem${appContext.context.appPage}`)
        if (element) {
            element.style.backgroundColor = "#560027"
            element.style.borderRadius = "16pt"
        }
    })

    const Root = styled("div", `ul { padding: 8pt 16pt } .item { font-size: 1.35rem }`)
    const item = classNames("pure-menu-item item")
    return (<Root className="pure-menu pure-menu-horizontal pure-menu-scrollable">
        <ul className="pure-menu-list">
            <li className={item}>
                <a href="home" className="pure-menu-link" id="MenuItemHome">Home</a>
            </li>
            <li className={item}>
                <a href="files" className="pure-menu-link" id="MenuItemFiles">Files</a>
            </li>
            <li className={item}>
                <a href="settings" className="pure-menu-link" id="MenuItemSettings">Settings</a>
            </li>
            <li class={item}>
                <a href="logout" className="pure-menu-link">Log Out</a>
            </li>
        </ul>
    </Root>)
}
