import {Element, FC} from "../../Utils"
import styled from "preact-css-styled"
import {useContext, useEffect} from "preact/compat"
import * as M from "materialize-css"
import classNames from "classnames"
import {ThemeContext, ThemeContextObject} from "../Theme"

export const Header: FC = (props, context): Element => {

    const theme: ThemeContextObject = useContext<ThemeContextObject>(ThemeContext)

    useEffect(() => {
        M.Sidenav.init(document.querySelectorAll(".sidenav"))
    })

    const Root = styled("header",
        `
.list-inline {
text-align: center;
display: flex;
flex-flow: row wrap;
justify-content: space-between;
align-items: flex-start;
align-content: flex-start;
}
.list-inline > li {
display: inline-block;
margin-bottom: 6pt;
}`)

    const onClick = () => {
        theme.update({primary: "pink"})
    }

    return (<Root>
        <div className="navbar-fixed">
            <nav class={classNames("nav-extended z-depth-3", theme.theme.primary)}
                 style={{borderBottomLeftRadius: "16pt", borderBottomRightRadius: "16pt"}}>
                <div class="nav-wrapper container">
                    <a class="brand-logo center" onClick={onClick}>Home</a>
                    <a href="" data-target="navmenu" class="sidenav-trigger"><i className="material-icons">menu</i></a>
                </div>
                <div className="nav-content">
                    <ul className="list-inline container">
                        <li>69% (3.8V)</li>
                        <li>04:20:42 Friday 20 April 2022</li>
                        <li>Log Out</li>
                    </ul>
                </div>
            </nav>
        </div>

        <ul class="sidenav sidenav-fixed" id="navmenu">
            <li class="logo"></li>
            <li>
                <a href="home">
                    <h4>Home</h4>
                </a>
            </li>
            <li>
                <a href="files">
                    <h4>Files</h4>
                </a>
            </li>
            <li>
                <a href="settings">
                    <h4>Settings</h4>
                </a>
            </li>
        </ul>


    </Root>)
}