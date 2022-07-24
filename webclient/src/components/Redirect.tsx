import {FC, Element} from "../Utils"
import {useEffect} from "preact/compat"
import {route} from "preact-router"

export interface RedirectProps {
    to: string
}

export const Redirect: FC<RedirectProps> = (props): Element | null => {
    useEffect(() => {
        route(props.to, true)
    }, [])
    return null
}