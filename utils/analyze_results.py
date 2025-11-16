#!/usr/bin/env python3


import pandas as pd
import matplotlib.pyplot as plt
import re
import sys
import os

def load_and_process_csv(csv_file):
    """Загружает CSV файл и группирует данные по имени файла."""
    if not os.path.exists(csv_file):
        print(f"Ошибка: файл {csv_file} не найден")
        sys.exit(1)
    
    df = pd.read_csv(csv_file)
    
    required_columns = [
        'file_name', 'score_first_step', 'time_first_step', 'distance_first_step',
        'score_second_step', 'time_second_step', 'distance_second_step'
    ]
    
    missing_columns = [col for col in required_columns if col not in df.columns]
    if missing_columns:
        print(f"Ошибка: отсутствуют необходимые колонки: {missing_columns}")
        sys.exit(1)
    
    numeric_columns = required_columns[1:]  # все кроме file_name
    grouped = df.groupby('file_name')[numeric_columns].mean().reset_index()
    
    grouped['file_number'] = grouped['file_name'].apply(
        lambda x: int(re.search(r'(\d+)', str(x)).group(1)) if re.search(r'(\d+)', str(x)) else 0
    )
    grouped = grouped.sort_values('file_number')
    
    return grouped

def plot_metric(ax, x, y1, y2, ylabel, title, colors=('blue', 'green')):
    """Универсальная функция для построения графика метрики."""
    ax.plot(x, y1, marker='o', linewidth=2, markersize=8, color=colors[0], 
            label='Первый шаг', alpha=0.8)
    ax.plot(x, y2, marker='s', linewidth=2, markersize=8, color=colors[1], 
            label='Второй шаг', alpha=0.8)
    ax.set_xlabel('Номер файла', fontsize=12)
    ax.set_ylabel(ylabel, fontsize=12)
    ax.set_title(title, fontsize=13, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend()

def calculate_improvements(df):
    """Вычисляет абсолютное и процентное улучшение."""
    # Абсолютное улучшение
    df['score_improvement'] = df['score_second_step'] - df['score_first_step']
    df['time_improvement'] = df['time_second_step'] - df['time_first_step']
    df['distance_improvement'] = df['distance_second_step'] - df['distance_first_step']
    
    # Процентное улучшение
    df['score_improvement_pct'] = (df['score_improvement'] / df['score_first_step'].abs()) * 100
    df['time_improvement_pct'] = ((df['time_first_step'] - df['time_second_step']) / df['time_first_step']) * 100
    df['distance_improvement_pct'] = ((df['distance_first_step'] - df['distance_second_step']) / df['distance_first_step']) * 100
    
    return df

def plot_all_results(df):
    plt.style.use('seaborn-v0_8-darkgrid' if 'seaborn-v0_8-darkgrid' in plt.style.available else 'default')
    
    x = df['file_number'].values
    
    # Графики для score и time
    fig1, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig1.suptitle('Результаты экспериментов (средние значения)', fontsize=16, fontweight='bold')
    
    plot_metric(axes[0, 0], x, df['score_first_step'], df['score_second_step'],
                'Целевая функция', 'Целевая функция')
    plot_metric(axes[0, 1], x, df['time_first_step'], df['time_second_step'],
                'Время', 'Время', colors=('orange', 'red'))
    plot_metric(axes[1, 0], x, df['distance_first_step'], df['distance_second_step'],
                'Дистанция', 'Дистанция пути', colors=('blue', 'purple'))
    
    # График абсолютных улучшений
    axes[1, 1].bar(x, df['score_improvement'].values, color='green', alpha=0.7)
    axes[1, 1].set_xlabel('Номер файла', fontsize=12)
    axes[1, 1].set_ylabel('Улучшение', fontsize=12)
    axes[1, 1].set_title('Абсолютное улучшение целевой функции', fontsize=13, fontweight='bold')
    axes[1, 1].grid(True, alpha=0.3, axis='y')
    axes[1, 1].axhline(y=0, color='black', linestyle='--', linewidth=1)
    
    plt.tight_layout()
    plt.savefig('experiment_results.png', dpi=300, bbox_inches='tight')
    print("График сохранен в файл: experiment_results.png")

def plot_percentage_improvements(df):
    """Строит графики процентного улучшения."""
    plt.style.use('seaborn-v0_8-darkgrid' if 'seaborn-v0_8-darkgrid' in plt.style.available else 'default')
    
    x = df['file_number'].values
    
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    fig.suptitle('Процентное улучшение после второго шага', fontsize=16, fontweight='bold')
    
    # График улучшения целевой функции
    axes[0].bar(x, df['score_improvement_pct'].values, color='green', alpha=0.7)
    axes[0].set_xlabel('Номер файла', fontsize=12)
    axes[0].set_ylabel('Улучшение (%)', fontsize=12)
    axes[0].set_title('Целевая функция', fontsize=13, fontweight='bold')
    axes[0].grid(True, alpha=0.3, axis='y')
    axes[0].axhline(y=0, color='black', linestyle='--', linewidth=1)
    
    # График улучшения времени
    axes[1].bar(x, df['time_improvement_pct'].values, color='orange', alpha=0.7)
    axes[1].set_xlabel('Номер файла', fontsize=12)
    axes[1].set_ylabel('Улучшение (%)', fontsize=12)
    axes[1].set_title('Время', fontsize=13, fontweight='bold')
    axes[1].grid(True, alpha=0.3, axis='y')
    axes[1].axhline(y=0, color='black', linestyle='--', linewidth=1)
    
    # График улучшения дистанции
    axes[2].bar(x, df['distance_improvement_pct'].values, color='purple', alpha=0.7)
    axes[2].set_xlabel('Номер файла', fontsize=12)
    axes[2].set_ylabel('Улучшение (%)', fontsize=12)
    axes[2].set_title('Дистанция', fontsize=13, fontweight='bold')
    axes[2].grid(True, alpha=0.3, axis='y')
    axes[2].axhline(y=0, color='black', linestyle='--', linewidth=1)
    
    plt.tight_layout()
    plt.savefig('experiment_results_improvements.png', dpi=300, bbox_inches='tight')
    print("График процентного улучшения сохранен в файл: experiment_results_improvements.png")

def print_separator():
    print("\n" + "-"*80)

def print_statistics(df):
    cols = ['file_name', 'score_first_step', 'time_first_step', 'distance_first_step',
            'score_second_step', 'time_second_step', 'distance_second_step']
    
    print_separator()
    print("Avg значения по файлам:\n")
    print(df[cols].to_string(index=False))
    
    print_separator()
    print("Абсолютное улучшение после второго шага:\n")
    improvement_cols = ['file_name', 'score_improvement', 'time_improvement', 'distance_improvement']
    print(df[improvement_cols].to_string(index=False))
    
    print_separator()
    print("Процентное улучшение после второго шага:\n")
    improvement_pct_cols = ['file_name', 'score_improvement_pct', 'time_improvement_pct', 'distance_improvement_pct']
    df_display = df[improvement_pct_cols].copy()
    df_display['score_improvement_pct'] = df_display['score_improvement_pct'].apply(lambda x: f"{x:.2f}%")
    df_display['time_improvement_pct'] = df_display['time_improvement_pct'].apply(lambda x: f"{x:.2f}%")
    df_display['distance_improvement_pct'] = df_display['distance_improvement_pct'].apply(lambda x: f"{x:.2f}%")
    print(df_display.to_string(index=False))
    
    print_separator()
    print("Среднее процентное улучшение:")
    print(f"  Целевая функция: {df['score_improvement_pct'].mean():.2f}%")
    print(f"  Время: {df['time_improvement_pct'].mean():.2f}%")
    print(f"  Дистанция: {df['distance_improvement_pct'].mean():.2f}%")
    print_separator()

def main():
    if len(sys.argv) < 2:
        print("Использование: python3 analyze_results.py <csv>")
        print("Пример: python3 analyze_results.py results.csv")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    
    df = load_and_process_csv(csv_file)
    
    df = calculate_improvements(df)
    
    print_statistics(df)
    
    plot_all_results(df)
    plot_percentage_improvements(df)
    

if __name__ == "__main__":
    main()
