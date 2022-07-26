import {FC, JSXElement} from "../Utils"
import {useEffect} from "preact/hooks"
import {route} from "preact-router"

export interface RedirectProps {
    to: string
}

export const Redirect: FC<RedirectProps> = (props): JSXElement | null => {
    useEffect(() => {
        route(props.to, true)
    }, [])
    return null
}