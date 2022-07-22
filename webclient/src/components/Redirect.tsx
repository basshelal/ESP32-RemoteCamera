import {FunctionComponent, JSX} from "preact"
import {useEffect} from "preact/compat"
import {route} from "preact-router"

export interface RedirectProps {
    to: string
}

export const Redirect: FunctionComponent<RedirectProps> = (props): JSX.Element | null => {
    useEffect(() => {
        route(props.to, true)
    }, [])
    return null
}