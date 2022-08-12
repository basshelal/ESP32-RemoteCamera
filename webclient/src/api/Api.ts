import {ApiBatteryResponse, ApiError, ApiLogResponse} from "./Types"
import {Constants} from "../Utils"

export class Api {
    private static join(...paths: Array<string>): string {
        return paths.reduce((previous: string, current: string) => `${previous}/${current}`)
    }

    private static api(url: string): string {
        return this.join(Constants.ServerURLHost, "api", url)
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

    public static createLogWebSocket(): WebSocket {
        // TODO: Make the url a bit better
        const url = this.join(Constants.ServerURLHost, "socket", "log").replace("http", "ws")
        return new WebSocket(url)
    }

    public static async postPassword(): Promise<void> {
        const url: string = this.api("password")
        const response: Response = await fetch(url, {method: "POST"})
        if (!response.ok) {
            throw new ApiError(url, response)
        }
    }
}

