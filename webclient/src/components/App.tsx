import {FC, JSXElement} from "../Utils"
import {Home} from "./pages/Home"
import {NotFound} from "./pages/NotFound"
import {Header} from "./ui-elements/Header"
import {Files} from "./pages/Files"
import {Log} from "./pages/Log"
import {Settings} from "./pages/Settings"
import {LogIn} from "./pages/LogIn"
import {useLayoutEffect, useState} from "preact/hooks"
import Router, {RouterOnChangeArgs} from "preact-router"
import {Redirect} from "./Redirect"
import {AppContext, AppContextObject, AppContextType, AppPage, DefaultContext} from "./AppContext"

export const App: FC = (): JSXElement => {

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
        if (!!args.path) {
            const page: AppPage | undefined = AppPage.fromPath(args.path)
            if (!!page) {
                setAppContext({appPage: page})
            }
        }
    }

    const Top: FC = (): JSXElement | null => isLoginPage ? null : (<Header/>)

    return (<AppContext.Provider value={appContextObject}>
        <Top/>
        <Router onChange={routerOnChange}>
            <LogIn path={AppPage.LOGIN.path}/>
            <Home path={AppPage.HOME.path}/>
            <Files path={AppPage.FILES.path}/>
            <Log path={AppPage.LOG.path}/>
            <Settings path={AppPage.SETTINGS.path}/>
            <Redirect path="/" to={AppPage.HOME.path}/>
            <Redirect path={AppPage.pagePrefix} to={AppPage.HOME.path}/>
            <NotFound default/>
        </Router>
    </AppContext.Provider>)
}