import {createRef, FunctionComponent, JSX, RefObject} from "preact"

export type FC<P = {}> = FunctionComponent<P>
export  type JSXElement = JSX.Element

export class Constants {
    public static ServerURLHost: string = "192.168.0.30"
    public static readonly LogWebSocketURL: string = "wss://"
}