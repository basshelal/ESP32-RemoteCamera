import {FunctionComponent, JSX, RenderableProps} from "preact"
import {EffectCallback, useEffect} from "preact/hooks"

export type P<T> = RenderableProps<T>
export type FC<P = {}> = FunctionComponent<P>
export  type JSXElement = JSX.Element

export class Constants {
    public static ServerURLHost: string = "http://192.168.0.123"
    public static readonly LogWebSocketURL: string = "ws://"
}

export function useOnce(once: EffectCallback): void {
    useEffect(once, [])
}

export class Logger {
    public static info(...message: any) {console.log(...message)}

    public static error(...message: any) {console.error(...message)}

    public static warn(...message: any) {console.warn(...message)}

    public static jsonify(object: any): string {return JSON.stringify(object)}
}