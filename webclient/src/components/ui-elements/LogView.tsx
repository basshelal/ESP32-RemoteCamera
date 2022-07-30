import {FC, JSXElement} from "../../Utils"
import {MutableRef, useEffect, useLayoutEffect, useRef, useState} from "preact/hooks"
import {Api} from "../../api/Api"
import {CSSProperties} from "preact/compat"
import classNames from "classnames"

export interface LogViewProps {
    className?: string
}

export const LogView: FC<LogViewProps> = (props): JSXElement => {
    const [setupFinished, setSetupFinished] = useState<boolean>(false)
    const [fetchedInitial, setFetchedInitial] = useState<boolean>(false)
    const [lines, setLines] = useState<Array<string>>(["state initial line 0", "state initial line 1"])

    const bottomDivRef: MutableRef<HTMLDivElement> = useRef<HTMLDivElement>(document.getElementById("bottomDiv") as HTMLDivElement)
    const autoScrollingRef: MutableRef<boolean> = useRef<boolean>(true)
    const lastScrollRatio: MutableRef<number> = useRef<number>(0)

    const cleanup = () => {
    }

    useLayoutEffect(() => {
        if (!fetchedInitial) {
            Api.getLogLines()
                .then((result: Array<string>) => {
                    setLines((prevLines: Array<string>) => [...prevLines, ...result])
                })
                .catch((error) => {})
            setFetchedInitial(true)
            return cleanup
        }
    }, [fetchedInitial])

    useEffect(() => {
        if (!setupFinished) {
            const root = document.getElementById("logViewRoot")!
            Api.registerLinesEvent(root)
            root.addEventListener("newLogLines", ((event: CustomEvent) => {
                setLines((prevLines: Array<string>) => {
                    const result = [...prevLines, ...event.detail.lines]
                    if (result.length > 100) {
                         result.splice(0, 1)
                    }
                    return result
                })
            }) as EventListener)
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
            setSetupFinished(true)
        }
    }, [setupFinished])

    useEffect(() => {
        if (bottomDivRef.current && autoScrollingRef.current) {
             bottomDivRef.current.scrollIntoView({block: "end"})
        }
    })

    // TODO: Use websocket to get instant notifications from server about new entries,
    //  on first load just get all data from server (as much as it can hold), new entries will be added
    //  to webpage likely more than what the server can hold? or do we keep parity with server?
    //  depends on our implementation of websocket communication
    return (<div id="logViewRoot" className={classNames(props.className)}>
        {lines.map((line: string, index: number) => {
            return (<pre key={index}>{line}</pre>)
        })}
        <div id="bottomDiv" ref={bottomDivRef}/>
    </div>)
}