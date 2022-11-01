import {JSXElement, Logger, P, useOnce} from "../../Utils"
import {MutableRef, useRef, useState} from "preact/hooks"
import {Slider} from "./Slider"
import {CameraSettings, DefaultCameraSettings, ImageSize, imageSizeToString} from "../../api/Types"
import {Api} from "../../api/Api"

export interface LivePlayerProps {
}

const updateCameraSettingsDelayMillis = 500

export const LivePlayer = (props: P<LivePlayerProps>): JSXElement => {

    const imageRef: MutableRef<HTMLImageElement> = useRef<HTMLImageElement>(document.getElementById("livePlayerImage") as HTMLImageElement)
    const [cameraSettings, setCameraSettings] = useState<CameraSettings>(DefaultCameraSettings)
    const queuedCameraSettingsRef: MutableRef<CameraSettings> = useRef<CameraSettings>({})
    const updateSettingsTimeoutRef: MutableRef<number> = useRef<number>(0)
    const webSocketRef: MutableRef<WebSocket | null> = useRef<WebSocket | null>(null)

    useOnce(() => {
        imageRef.current = document.getElementById("livePlayerImage") as HTMLImageElement
        webSocketRef.current = Api.createCameraWebSocket()
        webSocketRef.current!.onerror = (event) => {
            Logger.error("Websocket error")
        }
        webSocketRef.current!.onmessage = async (messageEvent: MessageEvent) => {
            const blob = messageEvent.data as Blob
            const image = imageRef.current
            if (!!image) {
                image.src = URL.createObjectURL(blob)
            }
        }
        return () => { // cleanup
            if (webSocketRef.current?.readyState == WebSocket.OPEN) {
                webSocketRef.current?.close()
            }
        }
    })

    const updateCameraSettings = (newCameraSettings: CameraSettings) => {
        queuedCameraSettingsRef.current = {...queuedCameraSettingsRef.current, ...newCameraSettings}
        setCameraSettings((oldCameraSettings: CameraSettings) => {
            return {...oldCameraSettings, ...newCameraSettings}
        })
        if (updateSettingsTimeoutRef.current != 0) {
            window.clearTimeout(updateSettingsTimeoutRef.current)
        }
        updateSettingsTimeoutRef.current = window.setTimeout(() => {
            Api.postCameraSettings(queuedCameraSettingsRef.current)
            queuedCameraSettingsRef.current = {}
            updateSettingsTimeoutRef.current = 0
        }, updateCameraSettingsDelayMillis)
    }

    return (<div>
        <img id="livePlayerImage" ref={imageRef} style={{width: "90vw", height: "auto"}}/>

        <Slider id="framerate"
                label={`Framerate: ${cameraSettings.frameRate} Hz`}
                min={1} max={20} value={cameraSettings.frameRate}
                onInput={(value) => updateCameraSettings({frameRate: value})}/>

        <Slider id="imageSize"
                label={`Image Size: ${imageSizeToString(cameraSettings.imageSize)}`}
                min={ImageSize.IMAGE_SIZE_320x240} max={ImageSize.IMAGE_SIZE_2592x1944} value={cameraSettings.imageSize}
                onInput={(value) => updateCameraSettings({imageSize: value})}/>

        <Slider id="minutesUntilStandby"
                label={`Standby after minutes idle: ${cameraSettings.minutesUntilStandby}`}
                min={1} max={60} value={cameraSettings.minutesUntilStandby}
                onInput={(value) => updateCameraSettings({minutesUntilStandby: value})}/>

        <Slider id="saturation"
                label={`saturation: ${cameraSettings.saturation}`}
                min={-4} max={4} value={cameraSettings.saturation}
                onInput={(value) => updateCameraSettings({saturation: value})}/>

        <Slider id="brightness"
                label={`brightness: ${cameraSettings.brightness}`}
                min={-4} max={4} value={cameraSettings.brightness}
                onInput={(value) => updateCameraSettings({brightness: value})}/>

        <Slider id="contrast"
                label={`contrast: ${cameraSettings.contrast}`}
                min={-4} max={4} value={cameraSettings.contrast}
                onInput={(value) => updateCameraSettings({contrast: value})}/>

        <Slider id="exposure"
                label={`exposure: ${cameraSettings.exposure}`}
                min={-4} max={4} value={cameraSettings.exposure}
                onInput={(value) => updateCameraSettings({exposure: value})}/>

        <Slider id="sharpness"
                label={`sharpness: ${cameraSettings.sharpness}`}
                min={-4} max={4} value={cameraSettings.sharpness}
                onInput={(value) => updateCameraSettings({sharpness: value})}/>

        <Slider id="mirrorFlip"
                label={`mirrorFlip: ${cameraSettings.mirrorFlip}`}
                min={-4} max={4} value={cameraSettings.mirrorFlip}
                onInput={(value) => updateCameraSettings({mirrorFlip: value})}/>

        <Slider id="compressionQuality"
                label={`compressionQuality: ${cameraSettings.compressionQuality}`}
                min={-4} max={4} value={cameraSettings.compressionQuality}
                onInput={(value) => updateCameraSettings({compressionQuality: value})}/>
    </div>)
}