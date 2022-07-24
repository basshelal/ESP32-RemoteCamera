export class ApiError extends Error {

    constructor(url: string, response: Response) {
        super(`${url} returned ${response.status}: ${response.statusText}`)
    }
}

export interface BatteryInfo {
    percentage: number
    voltage: number
}