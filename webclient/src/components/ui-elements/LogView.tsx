import {FC, JSXElement, Logger, useOnce} from "../../Utils"
import {MutableRef, useEffect, useRef, useState} from "preact/hooks"
import {Api} from "../../api/Api"
import classNames from "classnames"
import {ApiLogResponse} from "../../api/Types"

export interface LogViewProps {
    className?: string
}

export const LogView: FC<LogViewProps> = (props): JSXElement => {
    const [lines, setLines] = useState<Array<string>>([])
    const bottomDivRef: MutableRef<HTMLDivElement> = useRef<HTMLDivElement>(document.getElementById("bottomDiv") as HTMLDivElement)
    const autoScrollingRef: MutableRef<boolean> = useRef<boolean>(true)
    const lastScrollRatio: MutableRef<number> = useRef<number>(0)
    const webSocketRef: MutableRef<WebSocket | null> = useRef<WebSocket | null>(null)

    useEffect(() => {
        if (autoScrollingRef.current) {
            bottomDivRef.current.scrollIntoView({block: "end"})
        }
    })

    useOnce(() => {
        Api.getLog().then((apiLogResponse: ApiLogResponse) => {
            const root = document.getElementById("logViewRoot")!
            webSocketRef.current = Api.createLogWebSocket()
            webSocketRef.current!.onerror = (event) => {
                Logger.error("Websocket error")
            }
            webSocketRef.current!.onmessage = (messageEvent: MessageEvent) => {
                setLines((prevLines: Array<string>) => {
                    return prevLines.concat([messageEvent.data])
                })
            }
            root.addEventListener("scroll", (event) => {
                const root = event.target as HTMLDivElement
                const scrollAmount = root.scrollTop + root.clientHeight
                const maxScrollAmount = root.scrollHeight
                const scrollRatio = scrollAmount / maxScrollAmount
                if (scrollRatio >= 0.99) { // reached bottom
                    autoScrollingRef.current = true
                } else if (scrollRatio <= lastScrollRatio.current) {
                    autoScrollingRef.current = false
                }
                lastScrollRatio.current = scrollRatio
            })
            setLines((prevLines: Array<string>) => {
                return [...prevLines, ...apiLogResponse.lines]
            })
        })
        return () => { // cleanup
            if (webSocketRef.current?.readyState == WebSocket.OPEN) {
                webSocketRef.current?.close()
            }
        }
    })

    return (<div id="logViewRoot" className={classNames(props.className)}>
        {lines.map((line: string, index: number) => {
            return (<pre style={{fontSize: "0.8em"}} key={index}>{line}</pre>)
        })}
        <div id="bottomDiv" ref={bottomDivRef}/>
    </div>)
}