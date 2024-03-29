import {FC, useOnce} from "../../Utils"
import styled from "preact-css-styled"
import {useState} from "preact/hooks"
import {ApiBatteryResponse} from "../../api/Types"
import {Api} from "../../api/Api"
import {LivePlayer} from "../ui-elements/LivePlayer"

export const Home: FC = () => {

    const [batteryInfo, setBatteryInfo] = useState<ApiBatteryResponse>()

    useOnce(() => {
        Api.getBattery().then((apiBatteryResponse: ApiBatteryResponse) => {
            setBatteryInfo(apiBatteryResponse)
        })
    })

    const Root = styled("main",
        ".main { margin-top: 4px } video { width: 100%; margin-left: auto; margin-right: auto }" +
        ".mono { font-family: 'Fira Code', monospace }")

    const BatteryInfo: FC = () => {
        return (<>
            {batteryInfo?.voltage}mV {batteryInfo?.percentage}% {batteryInfo?.isCharging ? "charging" : null}
        </>)
    }

    return (<Root>
        <div className="main">
            <div className="pure-g">
                <p className="pure-u-1-3 mono" style={{textAlign: "left"}}>
                    <BatteryInfo/>
                </p>
                <p className="pure-u-1-3 mono" style={{textAlign: "center"}}>Info: Info 1</p>
                <p className="pure-u-1-3 mono" style={{textAlign: "right"}}>Info: Info 2</p>
            </div>
            <LivePlayer/>
        </div>
    </Root>)
}