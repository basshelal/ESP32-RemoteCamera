import {P} from "../../Utils"

export interface SliderProps {
    id?: string
    label?: string
    value?: number
    min?: number
    max?: number
    step?: number
    onInput?: (value: number) => void
    onChanged?: (value: number) => void
}

export const Slider = (props: P<SliderProps>) => {

    const onInput = (event: Event) => {
        const value = (event.target as HTMLInputElement).valueAsNumber
        if (props.onInput) {
            props.onInput(value)
        }
    }

    return (<div>
        <label htmlFor={`${props.id}Slider`}>{props.label}</label>
        <input type="range" id={props.id} name={`${props.id}Slider`}
               min={props.min} max={props.max} value={props.value}
               step={props.step ? props.step : 1}
               onInput={onInput}/>
    </div>)
}