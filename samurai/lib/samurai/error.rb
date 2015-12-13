module SamurAI
	class SamurAIError < Exception; end

  class CanNotMoveField < SamurAIError
    def message
      "Can not move"
    end
  end
end
