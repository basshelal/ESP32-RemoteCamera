import {Context} from "preact"
import {createContext} from "preact/compat"

export interface Theme {
    primary: string
    accent: string
}

export const DefaultTheme: Theme = {
    primary: "pink darken-4",
    accent: "light-blue lighten-2"
}

export interface ThemeContextObject {
    theme: Theme,
    update: (newTheme: Partial<Theme>) => void
}

export const ThemeContext: Context<ThemeContextObject> = createContext<ThemeContextObject>({theme: DefaultTheme, update: () => {}})