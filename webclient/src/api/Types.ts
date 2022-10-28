export class ApiError extends Error {

    constructor(url: string, response: Response) {
        super(`${url} returned ${response.status}: ${response.statusText}`)
    }
}

export interface ApiLogResponse {
    lineCount: number,
    lines: Array<string>
}

export interface ApiBatteryResponse {
    voltage: number
    percentage: number
    isCharging: boolean
}

export enum ImageSize {
    IMAGE_SIZE_320x240 = 0,
    IMAGE_SIZE_640x480 = 1,
    IMAGE_SIZE_1024x768 = 2,
    IMAGE_SIZE_1280x960 = 3,
    IMAGE_SIZE_1600x1200 = 4,
    IMAGE_SIZE_2048x1536 = 5,
    IMAGE_SIZE_2592x1944 = 6
}

export function imageSizeToString(imageSize?: ImageSize): string {
    if (imageSize == undefined) return ""
    switch (imageSize) {
        case ImageSize.IMAGE_SIZE_320x240:
            return "320x240"
        case ImageSize.IMAGE_SIZE_640x480:
            return "640x480"
        case ImageSize.IMAGE_SIZE_1024x768:
            return "1024x768"
        case ImageSize.IMAGE_SIZE_1280x960:
            return "1280x960"
        case ImageSize.IMAGE_SIZE_1600x1200:
            return "1600x1200"
        case ImageSize.IMAGE_SIZE_2048x1536:
            return "2048x1536"
        case ImageSize.IMAGE_SIZE_2592x1944:
            return "2592x1944"
    }
}

export interface CameraSettings {
    frameRate?: number
    imageSize?: ImageSize
    minutesUntilStandby?: number
    saturation?: number
    brightness?: number
    contrast?: number
    exposure?: number
    sharpness?: number
    mirrorFlip?: number
    compressionQuality?: number
    isRecording?: boolean
}

export const DefaultCameraSettings: CameraSettings = {
    frameRate: 10,
    imageSize: ImageSize.IMAGE_SIZE_1024x768,
    minutesUntilStandby: 5,
    saturation: 0,
    brightness: 0,
    contrast: 0,
    exposure: 0,
    sharpness: 0,
    mirrorFlip: 0,
    compressionQuality: 0,
    isRecording: false
}