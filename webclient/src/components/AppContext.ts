import {Context, createContext} from "preact"

export type AppPage = "Home" | "Files" | "Log" | "Settings" | "LogIn" | "NotFound"

export interface AppContextType {
    appPage: AppPage
}

export const DefaultContext: AppContextType = {
    appPage: "Home"
}

export interface AppContextObject {
    context: AppContextType,
    update: (newContext: Partial<AppContextType>) => void
}

export const AppContext: Context<AppContextObject> = createContext<AppContextObject>({
    context: DefaultContext,
    update: () => {}
})