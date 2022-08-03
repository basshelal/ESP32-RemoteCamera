import {Context, createContext} from "preact"
import {Home} from "./pages/Home"

export class AppPage {
    private constructor(public name: string,
                        public path: string) {
    }

    public static readonly pagePrefix: string = "/pages"
    public static readonly HOME: AppPage = new AppPage("Home", `${this.pagePrefix}/home`)
    public static readonly FILES: AppPage = new AppPage("Files", `${this.pagePrefix}/files`)
    public static readonly LOG: AppPage = new AppPage("Log", `${this.pagePrefix}/log`)
    public static readonly SETTINGS: AppPage = new AppPage("Settings", `${this.pagePrefix}/settings`)
    public static readonly LOGIN: AppPage = new AppPage("LogIn", `${this.pagePrefix}/login`)
    public static readonly NOT_FOUND: AppPage = new AppPage("NotFound", `${this.pagePrefix}/notfound`)
    public static readonly allAppPages: Array<AppPage> = [this.HOME, this.FILES, this.LOG, this.SETTINGS, this.LOGIN, this.NOT_FOUND]

    public static fromPath(path: string): AppPage | undefined {
        return this.allAppPages.find((it) => it.path === path)
    }
}

export interface AppContextType {
    appPage: AppPage
}

export const DefaultContext: AppContextType = {
    appPage: AppPage.NOT_FOUND
}

export interface AppContextObject {
    context: AppContextType,
    update: (newContext: Partial<AppContextType>) => void
}

export const AppContext: Context<AppContextObject> = createContext<AppContextObject>({
    context: DefaultContext,
    update: () => {}
})