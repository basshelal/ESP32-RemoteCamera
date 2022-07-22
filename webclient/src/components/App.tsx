import {FunctionComponent, JSX} from "preact"
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

export interface AppProps {
}

export const App: FunctionComponent<AppProps> = (): JSX.Element => {

    const [isLoginPage, setIsLoginPage] = useState<boolean>(false)

    useLayoutEffect(() => {
        if (document.URL.endsWith("/login")) {
            setIsLoginPage(true)
        } else {
            setIsLoginPage(false)
        }
    }, [document.URL])

    const Frame: FunctionComponent = (): JSX.Element | null => isLoginPage ? null :
        (<>
            <Header/>
            <Menu/>
        </>)

    return (<>
        <Frame/>
        <Router>
            <LogIn path="/login"/>
            <Home path="/home"/>
            <Files path="/files"/>
            <Settings path="/settings"/>
            <Redirect path="/" to="/home"/>
            <NotFound default/>
        </Router>
    </>)
}