import matplotlib.pyplot as plt
import sys

reward = [5000000000, 2500000000, 1250000000, 625000000, 312500000, 156250000, 78125000, 39062500, 19531250]
def btc(value):
    return '{}.{:08d}'.format(value // 100000000, value % 100000000)

for week in range(1, 10000):
    sys.stdin = open('analyzed_temp/week{}'.format(week), 'rt')

    block_start, block_end = map(int, input().split())
    tx_count = int(input())
    actual_sum = 0
    for i in range(block_end + 1):
        actual_sum += reward[i // 210000]
    utxo_sum = int(input())
    utxo3_sum = int(input())
    utxo5_sum = int(input())
    utxo10_sum = int(input())

    utxo_cnt = int(input())
    utxo3_cnt = int(input())
    utxo5_cnt = int(input())
    utxo10_cnt = int(input())

    burned = []
    sz = int(input())
    for i in range(sz):
        value, block = map(int, input().split())
        burned.append((value, block))

    john = []
    sz = int(input())
    for i in range(sz):
        value, timestamp, block = map(int, input().split())
        john.append((value, timestamp, block))

    total_supply = 2099999997690000
    total_mined = utxo_sum + utxo3_sum + utxo5_sum + utxo10_sum
    disappeared = actual_sum - total_mined
    total_supply -= disappeared

    label = ['Mined', '-']
    color = ['#bbbbbb', '#eeeeee']
    fontcolor = ['#000000', '#333333']
    data = [0, total_mined, total_supply - total_mined]
    total = sum(data)
    for i in range(1, len(data)):
        data[i] += data[i - 1]
    for i in range(len(data)):
        data[i] /= total

    fig, ax = plt.subplots(figsize=(20, 1))
    ax.xaxis.set_visible(False)
    ax.yaxis.set_visible(False)
    ax.axis("off")
    ax.set_xlim(0, 1)
    ax.set_ylim(0.95, 1.25)
    for i in range(len(data) - 1):
        if label[i] != '-':
            ax.barh(1, data[i + 1] - data[i], left = data[i], height = 0.1, label = label[i], color = color[i])
        else:
            ax.barh(1, data[i + 1] - data[i], left = data[i], height = 0.1, color = color[i])
        if fontcolor[i] != '-':
            ax.text((data[i] + data[i + 1]) / 2, 1, str(round((data[i + 1] - data[i]) * 100, 2)), ha='center', va='center', color = fontcolor[i])
    plt.legend(loc = 'upper left', ncol=len(label))
    plt.savefig('images/mined_week{}.png'.format(week))

    label = ['>10year', '>5year', '>3year', '-']
    color = ['#222222', '#666666', '#bbbbbb', '#eeeeee']
    fontcolor = ['#ffffff', '#ffffff', '#222222', '#000000']
    data = [0, utxo10_sum, utxo5_sum, utxo3_sum]
    data.append(total_mined - sum(data))
    total = sum(data)
    for i in range(1, len(data)):
        data[i] += data[i - 1]
    for i in range(len(data)):
        data[i] /= total

    fig, ax = plt.subplots(figsize=(20, 1))
    ax.xaxis.set_visible(False)
    ax.yaxis.set_visible(False)
    ax.axis("off")
    ax.set_xlim(0, 1)
    ax.set_ylim(0.95, 1.25)
    for i in range(len(data) - 1):
        if label[i] != '-':
            ax.barh(1, data[i + 1] - data[i], left = data[i], height = 0.1, label = label[i], color = color[i])
        else:
            ax.barh(1, data[i + 1] - data[i], left = data[i], height = 0.1, color = color[i])
        if fontcolor[i] != '-' and abs(data[i + 1] - data[i]) > 1e-7:
            ax.text((data[i] + data[i + 1]) / 2, 1, str(round((data[i + 1] - data[i]) * 100, 2)), ha='center', va='center', color = fontcolor[i])
    plt.legend(loc = 'upper left', ncol=len(label))
    plt.savefig('images/year_week{}.png'.format(week))

    f = open('report/week{}.md'.format(week), 'wt')

    print('# Bitcoin Week {}\n'.format(week), file = f)
    print('Block number: {}~{}\n'.format(block_start, block_end), file = f)
    print('The number of transaction on this week: {}\n'.format(tx_count), file = f)
    print('Total utxo: {}\n'.format(utxo_cnt + utxo3_cnt + utxo5_cnt + utxo10_cnt), file = f)

    print('![](../images/mined_week{}.png)\n'.format(week), file = f)
    print('Theoretical Total Supply: {} BTC\n'.format(btc(2099999997690000)), file = f)
    print('Permanently disappeared: {} BTC\n'.format(btc(disappeared)), file = f)
    print('Maximum Possible Total Supply: {} BTC\n'.format(btc(total_supply)), file = f)
    print('Current Supply: {} BTC ({:.3f}%)\n'.format(btc(total_mined), total_mined / total_supply * 100), file = f)

    print('![](../images/year_week{}.png)\n\n'.format(week), file = f)
    print('Current Supply: {} BTC ({:.3f}%)\n'.format(btc(total_mined), total_mined / total_mined * 100), file = f)
    print('More than 3 year: {} BTC ({:.3f}%)\n'.format(btc(utxo3_sum), utxo3_sum / total_mined * 100), file = f)
    print('More than 5 year: {} BTC ({:.3f}%)\n'.format(btc(utxo5_sum), utxo5_sum / total_mined * 100), file = f)
    print('More than 10 year: {} BTC ({:.3f}%)\n'.format(btc(utxo10_sum), utxo10_sum / total_mined * 100), file = f)

    print('# Remarks\n', file = f)
    if burned:
        print('## Permanently Disappeared BTC\n', file = f)
        for value, block in burned:
            print('{} satoshi disappeared on block {}\n'.format(value, block), file = f)

    if john:
        print('## Permanently Disappeared BTC\n', file = f)
        for value, timestamp, block in john:
            print('{} BTC was used in {} years and {} days\n'.format(btc(value), timestamp // (365 * 24 * 60 * 60), (timestamp // (24 * 60 * 60)) % 365))

    f.close()
