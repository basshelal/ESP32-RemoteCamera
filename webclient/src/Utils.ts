import {FunctionComponent, JSX} from "preact"
import {EffectCallback, Inputs, useEffect} from "preact/hooks"

export type FC<P = {}> = FunctionComponent<P>
export  type JSXElement = JSX.Element

export class Constants {
    public static ServerURLHost: string = "http://192.168.0.123"
    public static readonly LogWebSocketURL: string = "wss://"
}

export function useOnce(once: EffectCallback) : void {
    useEffect(once, [false])
}