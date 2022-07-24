import {Element, FC} from "../Utils"
import Router from "preact-router"
import {Home} from "./pages/Home"
import {Redirect} from "./Redirect"
import {NotFound} from "./pages/NotFound"
import {Header} from "./ui-elements/Header"
import {Menu} from "./ui-elements/Menu"
import {Files} from "./pages/Files"
import {Settings} from "./pages/Settings"
import {useLayoutEffect, useState} from "preact/compat"
import {LogIn} from "./pages/LogIn"
import {DefaultTheme, Theme, ThemeContext, ThemeContextObject} from "./Theme"

export interface AppProps {
}

export const App: FC<AppProps> = (): Element => {

    const [theme, setTheme] = useState<Theme>(DefaultTheme)
    const [isLoginPage, setIsLoginPage] = useState<boolean>(false)

    const themeContextObject: ThemeContextObject = {
        theme: theme,
        update: function (newTheme: Partial<Theme>): void {
            setTheme((oldTheme: Theme): Theme => {
                return {...oldTheme, ...newTheme}
            })
        }
    }

    useLayoutEffect(() => {
        if (document.URL.endsWith("/login")) {
            setIsLoginPage(true)
        } else {
            setIsLoginPage(false)
        }
    }, [document.URL])

    const Frame: FC = (): Element | null => isLoginPage ? null :
        (<>
            <Header/>
        </>)

    return (<ThemeContext.Provider value={themeContextObject}>
        <Frame/>
        <Router>
            <LogIn path="/login"/>
            <Home path="/home"/>
            <Files path="/files"/>
            <Settings path="/settings"/>
            <Redirect path="/" to="/home"/>
            <NotFound default/>
        </Router>
    </ThemeContext.Provider>)
}