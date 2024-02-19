from scipy.stats import poisson

p_less_than_8 = poisson.cdf(7, mu=1)
print(p_less_than_8)
p_at_least_8 = 1 - p_less_than_8
print(p_at_least_8)