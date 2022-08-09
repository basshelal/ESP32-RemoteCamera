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
    percentage: number
    voltage: number
}