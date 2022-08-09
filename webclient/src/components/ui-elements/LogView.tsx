import {FC, JSXElement, useOnce} from "../../Utils"
import {MutableRef, useEffect, useRef, useState} from "preact/hooks"
import {Api} from "../../api/Api"
import classNames from "classnames"
import {ApiLogResponse} from "../../api/Types"

export interface LogViewProps {
    className?: string
}

export const LogView: FC<LogViewProps> = (props): JSXElement => {
    const [lines, setLines] = useState<Array<string>>([])

    const setupFinishedRef: MutableRef<boolean> = useRef<boolean>(false)
    const bottomDivRef: MutableRef<HTMLDivElement> = useRef<HTMLDivElement>(document.getElementById("bottomDiv") as HTMLDivElement)
    const autoScrollingRef: MutableRef<boolean> = useRef<boolean>(true)
    const lastScrollRatio: MutableRef<number> = useRef<number>(0)
    const webSocketRef: MutableRef<WebSocket | null> = useRef<WebSocket | null>(null)
    const renderCount = useRef(0)

    const scrollToBottom = () => {
        if (bottomDivRef.current) {
            bottomDivRef.current.scrollIntoView({block: "end"})
        }
    }

    useEffect(() => {
        if (autoScrollingRef.current) {
            scrollToBottom()
        }
        console.log(`renders ${++renderCount.current}`)
    })

    const refs = {
        setupFinished: setupFinishedRef,
        bottomDiv: bottomDivRef,
        autoScrolling: autoScrollingRef,
        lastScrollRatio: lastScrollRatio,
        webSocket: webSocketRef
    }
    let string: string = ""
    for (const key in refs) {
        // @ts-ignore
        string = string.concat(`${key}: ${refs[key].current}\n`)
    }
    console.log(`Refs:\n${string}`)

    useOnce(() => {
        console.log("Effect at " + new Date().valueOf())
        if (!setupFinishedRef.current) {
            Api.getLog().then((apiLogResponse: ApiLogResponse) => {
                const root = document.getElementById("logViewRoot")!
                Api.registerLinesEvent(root)
                webSocketRef.current = Api.createLogWebSocket()
                webSocketRef.current!.onerror = (event) => {
                    console.log("Websocket error")
                }
                webSocketRef.current!.onmessage = (messageEvent: MessageEvent) => {
                    console.log(`${new Date().valueOf()}: ${messageEvent.data}`)
                    setLines((prevLines: Array<string>) => {
                        return prevLines.concat([messageEvent.data])
                    })
                }
                autoScrollingRef.current = true
                root.addEventListener("scroll", (event) => {
                    const root = event.target as HTMLDivElement
                    const scrollAmount = root.scrollTop + root.clientHeight
                    const maxScrollAmount = root.scrollHeight
                    const scrollRatio = Math.round(((scrollAmount / maxScrollAmount) + Number.EPSILON) * 100) / 100
                    if (scrollRatio >= 1) { // reached bottom
                        autoScrollingRef.current = true
                    } else if (scrollRatio <= lastScrollRatio.current) {
                        autoScrollingRef.current = false
                    }
                    lastScrollRatio.current = scrollRatio
                })
                setupFinishedRef.current = true
                setLines((prevLines: Array<string>) => {
                    console.log(`setLines count: ${prevLines.length + apiLogResponse.lines.length}`)
                    return [...prevLines, ...apiLogResponse.lines]
                })
            })
        }
        return () => { // cleanup
            console.log("Effect cleanup at " + new Date().valueOf())
            if (webSocketRef.current?.readyState == WebSocket.OPEN) {
                webSocketRef.current?.close()
            }
        }
    })

    return (<div id="logViewRoot" className={classNames(props.className)}>
        {lines.map((line: string, index: number) => {
            return (<pre key={index}>{line}</pre>)
        })}
        <div id="bottomDiv" ref={bottomDivRef}/>
    </div>)
}