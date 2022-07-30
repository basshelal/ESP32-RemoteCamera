import {ApiError, BatteryInfo} from "./Types"

export class Api {
    private static join(...paths: Array<string>): string {
        return paths.reduce((previous: string, current: string) => `${previous}/${current}`, "")
    }

    private static serverURL(): string {
        return document.location.host
    }

    private static api(url: string): string {
        return this.join(this.serverURL(), "api", url)
    }

    public static async getBattery(): Promise<BatteryInfo> {
        const url: string = this.api("battery")
        const response: Response = await fetch(url, {method: "GET"})
        if (response.ok) {
            const json: BatteryInfo = await response.json()
            return json
        } else {
            throw new ApiError(url, response)
        }
    }

    public static async getLogLines(): Promise<Array<string>> {
        return [`Initial Line 0`, `Initial Line 1`]

        const url: string = this.api("log")
        const response: Response = await fetch(url, {method: "GET"})
        if (response.ok) {
            const json: Array<string> = await response.json()
            return json
        } else {
            throw new ApiError(url, response)
        }
    }

    public static registerLinesEvent(root: HTMLElement): void {
        let count = 0
        setInterval(() => {
            root.dispatchEvent(new CustomEvent("newLogLines", {
                detail: {
                    lines: [`Line: ${count++}`]
                }
            }))
        }, 100)
    }

    public static async postPassword(): Promise<void> {
        const url: string = this.api("password")
        const response: Response = await fetch(url, {method: "POST"})
        if (!response.ok) {
            throw new ApiError(url, response)
        }
    }
}

