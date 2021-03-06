import matplotlib.pyplot as plt
import sys
from datetime import datetime

genesis = 1231006505
reward = [5000000000, 2500000000, 1250000000, 625000000, 312500000, 156250000, 78125000, 39062500, 19531250]
def btc(value):
    return '{}.{:08d}'.format(value // 100000000, value % 100000000)

for week in range(530, 10000):
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
    plt.subplots_adjust(left = 0.03, right = 0.97)
    plt.savefig('images/mined_week{:04d}.png'.format(week))

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
    plt.subplots_adjust(left = 0.03, right = 0.97)
    plt.savefig('images/year_week{:04d}.png'.format(week))

    f = open('report/week{:04d}.md'.format(week), 'wt')

    print('# Week {}\n'.format(week), file = f)
    if week != 1:
        print('[prev](week{:04d}.md) | '.format(week - 1), end = '', file = f)
    print('[next](week{:04d}.md)\n'.format(week + 1), file = f)
    print('- Block number: {}~{}\n'.format(block_start, block_end), file = f)
    print('- Date: {}~{}\n'.format(str(datetime.fromtimestamp(genesis + (week - 1) * 7 * 24 * 60 * 60)), str(datetime.fromtimestamp(genesis + week * 7 * 24 * 60 * 60 - 1))), file = f)
    print('- The number of transaction this week: {}\n'.format(tx_count), file = f)
    print('- Total utxo: {}\n'.format(utxo_cnt + utxo3_cnt + utxo5_cnt + utxo10_cnt), file = f)

    print('![](../images/mined_week{:04d}.png)\n'.format(week), file = f)
    print('- Theoretical Total Supply: {} BTC\n'.format(btc(2099999997690000)), file = f)
    print('- Permanently Disappeared: {} BTC\n'.format(btc(disappeared)), file = f)
    print('- Maximum Possible Total Supply: {} BTC\n'.format(btc(total_supply)), file = f)
    print('- Current Supply: {} BTC ({:.3f}%)\n'.format(btc(total_mined), total_mined / total_supply * 100), file = f)

    print('![](../images/year_week{:04d}.png)\n\n'.format(week), file = f)
    print('- Less than 3 years: {} BTC ({:.3f}%)\n'.format(btc(utxo_sum), utxo_sum / total_mined * 100), file = f)
    print('- More than 3 years: {} BTC ({:.3f}%)\n'.format(btc(utxo3_sum), utxo3_sum / total_mined * 100), file = f)
    print('- More than 5 years: {} BTC ({:.3f}%)\n'.format(btc(utxo5_sum), utxo5_sum / total_mined * 100), file = f)
    print('- More than 10 years: {} BTC ({:.3f}%)\n'.format(btc(utxo10_sum), utxo10_sum / total_mined * 100), file = f)

    print('# Remarks\n', file = f)
    if burned:
        print('## Permanently Disappeared BTC\n', file = f)
        for value, block in burned:
            print('- {} satoshi disappeared on block {}\n'.format(value, block), file = f)

    if john:
        print('## 10 years of Waiting\n', file = f)
        for value, timestamp, block in john:
            print('- {} BTC was used in {} years and {} days on block {}\n'.format(btc(value), timestamp // (365 * 24 * 60 * 60), (timestamp // (24 * 60 * 60)) % 365), block)

    f.close()
