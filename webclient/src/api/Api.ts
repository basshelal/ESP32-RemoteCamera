import {ApiBatteryResponse, ApiError, ApiLogResponse, CameraSettings} from "./Types"
import {Constants} from "../Utils"

export class Api {
    private static join(...paths: Array<string>): string {
        return paths.reduce((previous: string, current: string) => `${previous}/${current}`)
    }

    private static api(url: string): string {
        return this.join(Constants.ServerURLHost, "api", url)
    }

    private static ws(url: string): string {
        return this.join(Constants.ServerURLHost, "ws", url).replace("http", "ws")
    }

    public static async getBattery(): Promise<ApiBatteryResponse> {
        const url: string = this.api("battery")
        const response: Response = await fetch(url)
        if (response.ok) {
            return await response.json() as ApiBatteryResponse
        } else {
            throw new ApiError(url, response)
        }
    }

    public static async getLog(): Promise<ApiLogResponse> {
        const url: string = this.api("log")
        const response: Response = await fetch(url)
        if (response.ok) {
            return await response.json() as ApiLogResponse
        } else {
            throw new ApiError(url, response)
        }
    }

    public static createCameraWebSocket(): WebSocket {
        const url = this.ws("camera")
        return new WebSocket(url)
    }

    public static createLogWebSocket(): WebSocket {
        const url = this.ws("log")
        return new WebSocket(url)
    }

    public static async postCameraSettings(cameraSettings: CameraSettings) {
        const url = this.api("cameraSettings")
        const json = JSON.stringify(cameraSettings)
        console.log(json)
        const response: Response = await fetch(url, {method: "POST", body: json})
        if (!response.ok) {
            throw new ApiError(url, response)
        }
    }

    public static async getCameraSettings(): Promise<CameraSettings> {
        const url = this.api("cameraSettings")
        const response: Response = await fetch(url)
        if (response.ok) {
            return await response.json() as CameraSettings
        } else {
            throw new ApiError(url, response)
        }
    }

    public static async postPassword(): Promise<void> {
        const url: string = this.api("password")
        const response: Response = await fetch(url, {method: "POST"})
        if (!response.ok) {
            throw new ApiError(url, response)
        }
    }
}

