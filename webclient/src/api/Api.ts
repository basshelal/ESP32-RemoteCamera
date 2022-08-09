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
        const response: Response = await fetch(url, {method: "GET"})
        if (response.ok) {
            const json: ApiBatteryResponse = await response.json()
            return json
        } else {
            throw new ApiError(url, response)
        }
    }

    public static async getLog(): Promise<ApiLogResponse> {
        const url: string = this.api("log")
        const response: Response = await fetch(url, {method: "GET"})
        if (response.ok) {
            const json: ApiLogResponse = await response.json()
            return json
        } else {
            throw new ApiError(url, response)
        }
    }

    public static createLogWebSocket(): WebSocket {
        const url = this.join(Constants.ServerURLHost, "socket", "log").replace("http", "ws")
        const webSocket = new WebSocket(url)
        webSocket.onopen = (event: Event) => {
            console.log("Websocket opened!")
        }
        webSocket.onclose = (event: CloseEvent) => {
            console.log("Websocket closed!")
        }
        return webSocket
    }

    public static registerLinesEvent(root: HTMLElement): void {
        // let count = 0
        //
        // root.dispatchEvent(new CustomEvent("newLogLines", {
        //     detail: {
        //         lines: [`Line: ${count++}`]
        //     }
        // }))
    }

    public static async postPassword(): Promise<void> {
        const url: string = this.api("password")
        const response: Response = await fetch(url, {method: "POST"})
        if (!response.ok) {
            throw new ApiError(url, response)
        }
    }
}

