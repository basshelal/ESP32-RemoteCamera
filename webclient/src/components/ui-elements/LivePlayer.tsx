import {JSXElement, P, useOnce} from "../../Utils"
import {MutableRef, useRef, useState} from "preact/hooks"
import {Slider} from "./Slider"
import {CameraSettings, DefaultCameraSettings, ImageSize, imageSizeToString} from "../../api/Types"

export interface LivePlayerProps {
}

export const LivePlayer = (props: P<LivePlayerProps>): JSXElement => {

    const imageRef: MutableRef<HTMLImageElement> = useRef<HTMLImageElement>(document.getElementById("livePlayerImage") as HTMLImageElement)
    const [imageSettings, setImageSettings] = useState<CameraSettings>(DefaultCameraSettings)
    const [queuedImageSettings, setQueuedImageSettings] = useState<CameraSettings | null>(null)

    useOnce(() => {
        imageRef.current = document.getElementById("livePlayerImage") as HTMLImageElement
        setInterval(() => {
            if (imageRef.current) {
                const time = new Date().valueOf()
                imageRef.current.src = `http://192.168.0.123/api/camera?time=${time}`
            }
        }, 1000)
    })

    const updateImageSettings = (newImageSettings: CameraSettings) => {
        setImageSettings((oldImageSettings: CameraSettings) => {
            return {...oldImageSettings, ...newImageSettings}
        })
        // TODO: Queue settings changes to minimize server POST hits, use timers and intervals for this, maybe
        //  batching all changes into after 500ms of no more changes
        // Api.postCameraSettings(newImageSettings)
    }

    return (<div>
        <img id="livePlayerImage" ref={imageRef} style={{width: "90vw", height: "auto"}}/>

        <Slider id="framerate"
                label={`Framerate: ${imageSettings.frameRate} Hz`}
                min={1} max={20} value={imageSettings.frameRate}
                onInput={(value) => updateImageSettings({frameRate: value})}/>

        <Slider id="imageSize"
                label={`Image Size: ${imageSizeToString(imageSettings.imageSize)}`}
                min={ImageSize.IMAGE_SIZE_320x240} max={ImageSize.IMAGE_SIZE_2592x1944} value={imageSettings.imageSize}
                onInput={(value) => updateImageSettings({imageSize: value})}/>

        <Slider id="minutesUntilStandby"
                label={`Standby after minutes idle: ${imageSettings.minutesUntilStandby}`}
                min={1} max={60} value={imageSettings.minutesUntilStandby}
                onInput={(value) => updateImageSettings({minutesUntilStandby: value})}/>

        <Slider id="saturation"
                label={`saturation: ${imageSettings.saturation}`}
                min={-4} max={4} value={imageSettings.saturation}
                onInput={(value) => updateImageSettings({saturation: value})}/>

        <Slider id="brightness"
                label={`brightness: ${imageSettings.brightness}`}
                min={-4} max={4} value={imageSettings.brightness}
                onInput={(value) => updateImageSettings({brightness: value})}/>

        <Slider id="contrast"
                label={`contrast: ${imageSettings.contrast}`}
                min={-4} max={4} value={imageSettings.contrast}
                onInput={(value) => updateImageSettings({contrast: value})}/>

        <Slider id="exposure"
                label={`exposure: ${imageSettings.exposure}`}
                min={-4} max={4} value={imageSettings.exposure}
                onInput={(value) => updateImageSettings({exposure: value})}/>

        <Slider id="sharpness"
                label={`sharpness: ${imageSettings.sharpness}`}
                min={-4} max={4} value={imageSettings.sharpness}
                onInput={(value) => updateImageSettings({sharpness: value})}/>

        <Slider id="mirrorFlip"
                label={`mirrorFlip: ${imageSettings.mirrorFlip}`}
                min={-4} max={4} value={imageSettings.mirrorFlip}
                onInput={(value) => updateImageSettings({mirrorFlip: value})}/>

        <Slider id="compressionQuality"
                label={`compressionQuality: ${imageSettings.compressionQuality}`}
                min={-4} max={4} value={imageSettings.compressionQuality}
                onInput={(value) => updateImageSettings({compressionQuality: value})}/>
    </div>)
}