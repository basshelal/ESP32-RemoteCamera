import {FC, JSXElement} from "../../Utils"
import {useContext, useEffect} from "preact/hooks"
import {AppContext, AppContextObject, AppPage} from "../AppContext"
import styled from "preact-css-styled"
import classNames from "classnames"

export const Header: FC = (props, context): JSXElement => {

    const appContext: AppContextObject = useContext<AppContextObject>(AppContext)

    useEffect(() => {
        console.log(appContext.context.appPage.name)
        const element = document.getElementById(`MenuItem${appContext.context.appPage.name}`)
        if (element) {
            element.style.backgroundColor = "#560027"
            element.style.borderRadius = "16px"
        }
    })

    const Root = styled("div", `ul{padding:8px 16px}.item{font-size: 1.35rem}`)
    const item = classNames("pure-menu-item item")
    return (<Root className="pure-menu pure-menu-horizontal pure-menu-scrollable">
        <ul className="pure-menu-list">
            <li className={item}>
                <a href={AppPage.HOME.path} className="pure-menu-link" id="MenuItemHome">Home</a>
            </li>
            <li className={item}>
                <a href={AppPage.FILES.path} className="pure-menu-link" id="MenuItemFiles">Files</a>
            </li>
            <li className={item}>
                <a href={AppPage.LOG.path} className="pure-menu-link" id="MenuItemLog">Log</a>
            </li>
            <li className={item}>
                <a href={AppPage.SETTINGS.path} className="pure-menu-link" id="MenuItemSettings">Settings</a>
            </li>
            <li class={item}>
                <a href={AppPage.LOGIN.path} className="pure-menu-link">Log Out</a>
            </li>
        </ul>
    </Root>)
}
