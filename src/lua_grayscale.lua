function process(g, b, r, a, out_g, out_b, out_r, out_a, width, height)
	i = 0
	while i < height do
		j = 0
		while j < width do
			local Y = 0.299 * r:get(i, j) + 0.587 * g:get(i, j) + 0.114 * b:get(i, j)
			out_g:set(i, j, Y)
			out_b:set(i, j, Y)
			out_r:set(i, j, Y)
			out_a:set(i, j, a:get(i,j))
			j = j + 1
		end
		i = i + 1
	end
end