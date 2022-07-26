import {FC, JSXElement} from "../Utils"
import {Home} from "./pages/Home"
import {NotFound} from "./pages/NotFound"
import {Header} from "./ui-elements/Header"
import {Files} from "./pages/Files"
import {Settings} from "./pages/Settings"
import {LogIn} from "./pages/LogIn"
import {useLayoutEffect, useState} from "preact/hooks"
import Router, {RouterOnChangeArgs} from "preact-router"
import {Redirect} from "./Redirect"
import {AppContext, AppContextObject, AppContextType, AppPage, DefaultContext} from "./AppContext"

export const App: FC = (): JSXElement => {

    let appPage: AppPage
    const [appContext, setAppContext] = useState<AppContextType>(DefaultContext)
    const [isLoginPage, setIsLoginPage] = useState<boolean>(false)

    const appContextObject: AppContextObject = {
        context: appContext,
        update: function (newContext: Partial<AppContextType>): void {
            setAppContext((oldContext: AppContextType): AppContextType => {
                return {...oldContext, ...newContext}
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

    const routerOnChange = (args: RouterOnChangeArgs): void => {
        switch (args.path) {
            case "/login":
                appPage = "LogIn"
                break
            case "/home":
                appPage = "Home"
                break
            case "/files":
                appPage = "Files"
                break
            case "/settings":
                appPage = "Settings"
                break
            default:
                appPage = "NotFound"
        }
        setAppContext({appPage: appPage})
    }

    const Top: FC = (): JSXElement | null => isLoginPage ? null : (<Header/>)

    return (<AppContext.Provider value={appContextObject}>
        <Top/>
        <Router onChange={routerOnChange}>
            <LogIn path="/login"/>
            <Home path="/home"/>
            <Files path="/files"/>
            <Settings path="/settings"/>
            <Redirect path="/" to="/home"/>
            <NotFound default/>
        </Router>
    </AppContext.Provider>)
}