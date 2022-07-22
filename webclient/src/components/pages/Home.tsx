import {FunctionComponent, JSX} from "preact"

export interface HomeProps {}

export const Home: FunctionComponent<HomeProps> = (): JSX.Element => {

    return (<>
        <h1>Home</h1>
    </>)
}