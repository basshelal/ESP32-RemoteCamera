import {FC, JSXElement} from "../../Utils"
import {useEffect} from "preact/hooks"
import Hls, {Events, HlsConfig, ManifestParsedData, MediaAttachedData} from "hls.js/dist/hls.js"
import {HTMLAttributes} from "preact/compat"

export interface VideoPlayerProps {
    hlsOptions?: Partial<HlsConfig>
    videoOptions?: Partial<HTMLAttributes<HTMLVideoElement>>
}

export const VideoPlayer: FC<VideoPlayerProps> = (props): JSXElement => {
    const id = "video-player"
    const playlistUrl = "https://test-streams.mux.dev/x36xhzz/x36xhzz.m3u8"

    useEffect(() => {
        const videoElement = document.getElementById("video-player")! as HTMLVideoElement
        const hls = new Hls({
            ...props.hlsOptions,
            autoStartLoad: true
        })
        hls.attachMedia(videoElement)
        hls.on(Hls.Events.MEDIA_ATTACHED, (event: Events.MEDIA_ATTACHED, data: MediaAttachedData) => {
            console.log("Attached!")
        })
        hls.loadSource(playlistUrl)
        hls.on(Hls.Events.MANIFEST_PARSED, (event: Events.MANIFEST_PARSED, data: ManifestParsedData) => {
            console.log("Parsed!")
        })
    })

    return (<video {...props.videoOptions} id={id}>
    </video>)
}