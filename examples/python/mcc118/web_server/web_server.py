#!/usr/bin/env python
#  -*- coding: utf-8 -*-
"""
This example demonstrates a simple web server providing visualization of data
from a MCC118 DAQ HAT device for a single client.  It makes use of the Dash
Python framework for web-based interfaces and a plotly graph.  To install the
dependencies for this example, run:
   $ pip install dash dash-renderer dash-html-components dash-core-components
   $ pip install plotly

Running this example:
1. Start the server by running the web_server.py module in a terminal.
   $ ./web_server.py
2. Open a web browser on a device on the same network as the host device and
   enter http://<host>:8080 in the address bar,
   replacing <host> with the IP Address or hostname of the host device.

Stopping this example:
1. To stop the server press Ctrl+C in the terminal window where there server
   was started.
"""
import dash
import dash_core_components as dcc
import dash_html_components as html
from dash.dependencies import Input, Output, State, Event
import plotly.graph_objs as go
import json
import socket
import daqhats as hats

app = dash.Dash(__name__)

app.css.config.serve_locally = True
app.scripts.config.serve_locally = True

g_hat = None  # Global variable to retain MCC118 HAT object


def create_hat_selector():
    """
    Gets a list of available MCC118 devices and creates a corresponding
    dash-core-components Dropdown element for the user interface.

    Returns:
        A dash-core-components Dropdown object.
    """
    hat_list = hats.hat_list(filter_by_id=hats.HatIDs.MCC_118)
    hat_selection_options = []
    for hat in hat_list:
        # Create the label from the address and product name
        label = '{0}: {1}'.format(hat.address, hat.product_name)
        # Create the value by converting the descriptor to a JSON object
        option = {'label': label, 'value': json.dumps(hat._asdict())}
        hat_selection_options.append(option)

    selection = None
    if len(hat_selection_options) > 0:
        selection = hat_selection_options[0]['value']

    return dcc.Dropdown(id='hatSelector', options=hat_selection_options,
                        value=selection, clearable=False)


def init_chart_data(number_of_channels, number_of_samples):
    """
    Initializes the chart with the specified number of samples.

    Args:
        number_of_channels (int): The number of channels to be displayed.
        number_of_samples (int): The number of samples to be displayed.

    Returns:
        A JSON object containing the chart data.
    """
    samples = []
    for i in range(number_of_samples):
        samples.append(i)
    data = []
    for _ in range(number_of_channels):
        data.append([None]*number_of_samples)

    chart_data = {'data': data, 'samples': samples, 'sample_count': 0}

    return json.dumps(chart_data)


# Define the HTML layout for the user interface, consisting of
# dash-html-components and dash-core-components.
app.layout = html.Div([
    html.H1(
        children='MCC118 DAQ HAT Web Server Example',
        id='exampleTitle'
    ),
    html.Div([
        html.Div(
            id='rightContent',
            children=[
                dcc.Graph(id='stripChart'),
                html.Div(id='errorDisplay',
                         children='',
                         style={'font-weight': 'bold', 'color': 'red'})
            ], style={'width': '100%', 'box-sizing': 'border-box',
                      'float': 'left', 'padding-left': 320}
        ),
        html.Div(
            id='leftContent',
            children=[
                html.Label('Select a HAT...', style={'font-weight': 'bold'}),
                create_hat_selector(),
                html.Label('Sample Rate (Hz)',
                           style={'font-weight': 'bold', 'display': 'block',
                                  'margin-top': 10}),
                dcc.Input(id='sampleRate', type='number', max=100000.0,
                          step=1, value=1000.0,
                          style={'width': 100, 'display': 'block'}),
                html.Label('Samples to display',
                           style={'font-weight': 'bold',
                                  'display': 'block', 'margin-top': 10}),
                dcc.Input(id='samplesToDisplay', type='number', min=1,
                          max=1000, step=1, value=100,
                          style={'width': 100, 'display': 'block'}),
                html.Label('Active Channels',
                           style={'font-weight': 'bold', 'display': 'block',
                                  'margin-top': 10}),
                dcc.Checklist(
                    id='channelSelections',
                    options=[
                        {'label': 'Channel 0', 'value': 0},
                        {'label': 'Channel 1', 'value': 1},
                        {'label': 'Channel 2', 'value': 2},
                        {'label': 'Channel 3', 'value': 3},
                        {'label': 'Channel 4', 'value': 4},
                        {'label': 'Channel 5', 'value': 5},
                        {'label': 'Channel 6', 'value': 6},
                        {'label': 'Channel 7', 'value': 7},
                    ],
                    labelStyle={'display': 'block'},
                    values=[0]
                ),
                html.Button(
                    children='Configure',
                    id='startStopButton',
                    style={'width': 100, 'height': 35, 'text-align': 'center',
                           'margin-top': 10}
                ),
            ], style={'width': 320, 'box-sizing': 'border-box', 'padding': 10,
                      'position': 'absolute', 'top': 0, 'left': 0}
        ),
    ], style={'position': 'relative', 'display': 'block',
              'overflow': 'hidden'}),
    dcc.Interval(
        id='timer',
        interval=1000*60*60*24  # in milliseconds
    ),
    html.Div(
        id='chartData',
        style={'display': 'none'},
        children=init_chart_data(1, 1000)
    ),
    html.Div(
        id='status',
        style={'display': 'none'}
    ),
])


@app.callback(
    Output('status', 'children'),
    [Input('startStopButton', 'n_clicks')],
    [State('startStopButton', 'children'),
     State('hatSelector', 'value'),
     State('sampleRate', 'value'),
     State('samplesToDisplay', 'value'),
     State('channelSelections', 'values')]
)
def start_stop_click(n_clicks, button_label, hat_descriptor_json_str,
                     sample_rate_val, samples_to_display, active_channels):
    """
    A callback function to change the application status when the Configure,
    Start or Stop button is clicked.

    Args:
        n_clicks (int): Number of button clicks - triggers the callback.
        button_label (str): The current label on the button.
        hat_descriptor_json_str (str): A string representation of a JSON object
            containing the descriptor for the selected MCC118 DAQ HAT.
        sample_rate_val (float): The user specified sample rate value.
        samples_to_display (float): The number of samples to be displayed.
        active_channels ([int]): A list of integers corresponding to the user
            selected Active channel checkboxes.

    Returns (str):
        The new application status - "idle", "configured" or "running"

    """
    global g_hat
    output = 'idle'
    if n_clicks is not None and n_clicks > 0:
        if button_label == 'Configure':
            if (1 < samples_to_display <= 1000
                    and len(active_channels) > 0
                    and sample_rate_val < (100000 / len(active_channels))):
                # If configuring, create the hat object.
                if hat_descriptor_json_str:
                    hat_descriptor = json.loads(hat_descriptor_json_str)
                    g_hat = hats.mcc118(hat_descriptor['address'])
                    output = 'configured'
            else:
                output = 'error'
        elif button_label == 'Start':
            # If starting, call the a_in_scan_start function.
            sample_rate = float(sample_rate_val)
            channel_mask = 0x0
            for channel in active_channels:
                channel_mask |= 1 << channel
            hat = g_hat
            hat.a_in_scan_start(channel_mask, 10000,
                                sample_rate, hats.OptionFlags.CONTINUOUS)
            output = 'running'
        elif button_label == 'Stop':
            # If stopping, call the a_in_scan_stop and a_in_scan_cleanup
            # functions.
            hat = g_hat
            hat.a_in_scan_stop()
            hat.a_in_scan_cleanup()
            output = 'idle'

    return output


@app.callback(
    Output('timer', 'interval'),
    [Input('status', 'children'),
     Input('samplesToDisplay', 'value')],
    [State('channelSelections', 'values')]
)
def update_timer_interval(acq_state, samples_to_display, active_channels):
    """
    A callback function to update the timer interval for reading data when the
    application status changes.  The resulting interval is a product of the
    number of active channels and the number of samples to display per channel
    on the strip chart (with a minimum of 100 ms and maximum of 800 ms).

    Args:
        acq_state (str): The application state of "idle", "configured",
            "running" or "error" - triggers the callback.
        samples_to_display (float): The number of samples to be displayed.
        active_channels ([int]): A list of integers corresponding to the user
            selected Active channel checkboxes.

    Returns (int):
        The new timer interval in ms.
    """
    refresh_rate = int(len(active_channels) * samples_to_display)
    refresh_rate = 100 if refresh_rate < 100 else refresh_rate
    refresh_rate = 800 if refresh_rate > 800 else refresh_rate
    return refresh_rate if acq_state == 'running' else 1000*60*60*24


@app.callback(
    Output('hatSelector', 'disabled'),
    [Input('status', 'children')]
)
def disable_hat_selector_dropdown(acq_state):
    """
    A callback function to disable the HAT selector dropdown when the
    application status changes to running.
    """
    return True if acq_state == 'running' else False


@app.callback(
    Output('sampleRate', 'disabled'),
    [Input('status', 'children')]
)
def disable_sample_rate_input(acq_state):
    """
    A callback function to disable the sample rate input when the
    application status changes to running.
    """
    return True if acq_state == 'running' else False


@app.callback(
    Output('samplesToDisplay', 'disabled'),
    [Input('status', 'children')]
)
def disable_samples_to_display_input(acq_state):
    """
    A callback function to disable the number of samples to display input
    when the application status changes to running.
    """
    return True if acq_state == 'running' else False


@app.callback(
    Output('channelSelections', 'options'),
    [Input('status', 'children')]
)
def disable_sample_rate_input(acq_state):
    """
    A callback function to disable the active channel checkboxes when the
    application status changes to running.
    """
    options = []
    for channel in range(8):
        label = 'Channel ' + str(channel)
        disabled = True if acq_state == 'running' else False
        options.append({'label': label, 'value': channel, 'disabled': disabled})
    return options


@app.callback(
    Output('startStopButton', 'children'),
    [Input('status', 'children')]
)
def update_start_stop_button_name(acq_state):
    """
    A callback function to update the label on the button when the application
    status changes.

    Args:
        acq_state (str): The application state of "idle", "configured",
            "running" or "error" - triggers the callback.

    Returns (str):
        The new button label of "Configure", "Start" or "Stop"
    """
    output = 'Configure'
    if acq_state == 'configured':
        output = 'Start'
    elif acq_state == 'running':
        output = 'Stop'
    return output


@app.callback(
    Output('chartData', 'children'),
    [Input('status', 'children')],
    [State('chartData', 'children'),
     State('samplesToDisplay', 'value'),
     State('channelSelections', 'values')],
    [Event('timer', 'interval')]
)
def update_strip_chart_data(acq_state, chart_data_json_str,
                            samples_to_display_val, active_channels):
    """
    A callback function to update the chart data stored in the chartData HTML
    div element.  The chartData element is used to store the existing data
    values, which allows sharing of data between callback functions.  Global
    variables cannot be used to share data between callbacks (see
    https://dash.plot.ly/sharing-data-between-callbacks).

    Args:
        acq_state (str): The application state of "idle", "configured",
            "running" or "error" - triggers the callback.
        chart_data_json_str (str): A string representation of a JSON object
            containing the current chart data.
        samples_to_display_val (float): The number of samples to be displayed.
        active_channels ([int]): A list of integers corresponding to the user
            selected Active channel checkboxes.

    Returns (str):
        A string representation of a JSON object containing the updated chart
        data.
    """
    global g_hat
    updated_chart_data = chart_data_json_str
    samples_to_display = int(samples_to_display_val)
    num_channels = len(active_channels)
    if acq_state == 'running':
        hat = g_hat
        if hat is not None:
            chart_data = json.loads(chart_data_json_str)
            current_sample_count = int(chart_data['sample_count'])

            # By specifying -1 for the samples_per_channel parameter, the
            # timeout is ignored and all available data is read.
            read_result = hat.a_in_scan_read(-1, 0)
            num_samples_read = int(len(read_result.data) / num_channels)
            total_sample_count = current_sample_count + num_samples_read

            if ('hardware_overrun' not in chart_data.keys()
                    or not chart_data['hardware_overrun']):
                chart_data['hardware_overrun'] = read_result.hardware_overrun
            if ('buffer_overrun' not in chart_data.keys()
                    or not chart_data['buffer_overrun']):
                chart_data['buffer_overrun'] = read_result.buffer_overrun

            start_sample = current_sample_count
            if total_sample_count < samples_to_display:
                # If the total samples read is less than the samples to display
                # then update the data array with all of the new data.
                for i in range(num_samples_read):
                    for channel in range(num_channels):
                        sample = start_sample + i
                        data_index = i * num_channels + channel
                        value = read_result.data[data_index]
                        chart_data['data'][channel][sample] = value
            else:
                samples_to_append = num_samples_read
                if num_samples_read < samples_to_display:
                    # Determine how many of the currently displayed samples to
                    # keep and slice off the rest of the data.
                    keep = samples_to_display - num_samples_read
                    chart_data['samples'] = chart_data['samples'][(-1 * keep):]
                    for channel in range(num_channels):
                        chan_data = chart_data['data'][channel]
                        chart_data['data'][channel] = chan_data[(-1 * keep):]
                else:
                    # If the data read in this callback is enough the fill
                    # the strip chart, clear the data arrays.
                    start_sample = total_sample_count - samples_to_display
                    samples_to_append = samples_to_display
                    chart_data['samples'] = []
                    for channel in range(num_channels):
                        chart_data['data'][channel] = []

                # Append the new data.
                for sample in range(samples_to_append):
                    chart_data['samples'].append(start_sample + sample)
                    for channel in range(num_channels):
                        data_index = sample * num_channels + channel
                        value = read_result.data[data_index]
                        chart_data['data'][channel].append(value)

            # Update the total sample count.
            chart_data['sample_count'] = total_sample_count
            updated_chart_data = json.dumps(chart_data)
    elif acq_state == 'configured':
        # Clear the data in the strip chart when Configure is clicked.
        updated_chart_data = init_chart_data(num_channels, samples_to_display)

    return updated_chart_data


@app.callback(
    Output('stripChart', 'figure'),
    [Input('chartData', 'children')],
    [State('channelSelections', 'values')]
)
def update_strip_chart(chart_data_json_str, active_channels):
    """
    A callback function to update the strip chart display when new data is read.

    Args:
        chart_data_json_str (str): A string representation of a JSON object
            containing the current chart data - triggers the callback.
        active_channels ([int]): A list of integers corresponding to the user
            selected Active channel checkboxes.

    Returns (object):
        A figure object for a dash-core-components Graph, updated the most
        recently read data.
    """
    data = []
    xaxis_range = [0, 1000]
    chart_data = json.loads(chart_data_json_str)
    if 'samples' in chart_data and len(chart_data['samples']) > 0:
        xaxis_range = [min(chart_data['samples']), max(chart_data['samples'])]
    if 'data' in chart_data:
        data = chart_data['data']

    plot_data = []
    colors = ['#DD3222', '#FFC000', '#3482CB', '#FF6A00',
              '#75B54A', '#808080', '#6E1911', '#806000']
    # Update the serie data for each active channel.
    for channel in range(len(active_channels)):
        scatter_serie = go.Scatter(
            x=list(chart_data['samples']),
            y=list(data[channel]),
            name='Channel {0:d}'.format(active_channels[channel]),
            marker=go.Marker(color=colors[active_channels[channel]])
        )
        plot_data.append(scatter_serie)

    figure = {
        'data': plot_data,
        'layout': go.Layout(
            xaxis=dict(title='Samples', range=xaxis_range),
            yaxis=dict(title='Voltage'),
            margin={'l': 40, 'r': 40, 't': 50, 'b': 40, 'pad': 0},
            showlegend=True,
            title='Strip Chart'
        )
    }

    return figure


@app.callback(
    Output('errorDisplay', 'children'),
    [Input('chartData', 'children'),
     Input('status', 'children')],
    [State('hatSelector', 'value'),
     State('sampleRate', 'value'),
     State('samplesToDisplay', 'value'),
     State('channelSelections', 'values')]
)
def update_error_message(chart_data_json_str, acq_state, hat_selection,
                         sample_rate, samples_to_display, active_channels):
    """
    A callback function to display error messages.

    Args:
        chart_data_json_str (str): A string representation of a JSON object
            containing the current chart data - triggers the callback.
        acq_state (str): The application state of "idle", "configured",
            "running" or "error" - triggers the callback.
        hat_selection (str): A string representation of a JSON object
            containing the descriptor for the selected MCC118 DAQ HAT.
        sample_rate (float): The user specified sample rate value.
        samples_to_display (float): The number of samples to be displayed.
        active_channels ([int]): A list of integers corresponding to the user
            selected Active channel checkboxes.

    Returns (str):
        The error message to display.

    """
    error_message = ''
    if acq_state == 'running':
        chart_data = json.loads(chart_data_json_str)
        if ('hardware_overrun' in chart_data.keys()
                and chart_data['hardware_overrun']):
            error_message += 'Hardware overrun occurred; '
        if ('buffer_overrun' in chart_data.keys()
                and chart_data['buffer_overrun']):
            error_message += 'Buffer overrun occurred; '
    elif acq_state == 'error':
        num_active_channels = len(active_channels)
        print('Active channels:', num_active_channels)
        max_sample_rate = 100000
        if not hat_selection:
            error_message += 'Invalid HAT selection; '
        if num_active_channels <= 0:
            error_message += 'Invalid channel selection (min 1); '
        else:
            max_sample_rate = 100000 / len(active_channels)
        if sample_rate > max_sample_rate:
            error_message += 'Invalid Sample Rate (max: '
            error_message += str(max_sample_rate) + '); '
        if samples_to_display <= 1 or samples_to_display > 1000:
            error_message += 'Invalid Samples to display (range: 2-1000); '

    return error_message


def get_ip_address():
    """ Utility function to get the IP address of the device. """
    ip_address = '127.0.0.1'  # Default to localhost
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        s.connect(('1.1.1.1', 1))  # Does not have to be reachable
        ip_address = s.getsockname()[0]
    finally:
        s.close()

    return ip_address


if __name__ == '__main__':
    app.run_server(host=get_ip_address(), port=8080)
